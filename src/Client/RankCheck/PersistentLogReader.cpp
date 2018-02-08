#include <Client/RankCheck/PersistentLogReader.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Utils/DebugLog.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <utility>

static cfg::Int cfgMatchStartPos("rankcheck.persistentLogReader.currentOffset");
static cfg::List<cfg::Int> cfgRatingHistory("rankcheck.ratingHistory.buffer");
static cfg::Int cfgRatingHistoryBufferSize("rankcheck.ratingHistory.bufferSize");

PersistentLogReader::PersistentLogReader()
{
}

PersistentLogReader::~PersistentLogReader()
{
}

void PersistentLogReader::markMatchStart()
{
	matchStartPos = tell();
}

void PersistentLogReader::skipToMatchStart()
{
	if (!seek(matchStartPos))
	{
		debug() << "Failed to seek to match start!";
		ratingHistory.clear();

		if (!seekFromEnd(0))
		{
			debug() << "Failed to seek to file start!";
		}
		else
		{
			foundMatch = true;
		}
	}
	else
	{
		foundMatch = true;
	}
}

void PersistentLogReader::onReopenFile()
{
	foundMatch = false;
	resetPlayers();
}

void PersistentLogReader::onReopenFileSuccess()
{
	if (foundMatch)
	{
		emitCurrentData();
	}
}

void PersistentLogReader::onProcessLine(const std::string& line, bool withCallback)
{
	static const std::string ownSteamIDKey = "\tInitialised steam for appId: 204300 userId: ";
	static const std::string remotePlayerJoinKey = "\treceived onPlayerJoined, player:  ";
	static const std::string localPlayerJoinKey = "\tlocal onPlayerJoined, player:  ";
	static const std::string gameEndKey = "\tGame end request  ";
	static const std::string rankingScoreKey = "\tUploading new ranking score of  ";

	std::size_t found = 0;

	found = line.find(ownSteamIDKey);
	if (found != std::string::npos)
	{
		std::size_t foundSpace = line.find_first_of(' ', found);
		if (foundSpace != std::string::npos)
		{
			sf::Uint64 steamID = cStoUL(line.substr(found, foundSpace - found));
			if (localSteamID != steamID)
			{
				// TODO: handle rating history properly when switching accounts (per-account history buffer).
				localSteamID = steamID;
			}
		}
	}

	found = line.find(remotePlayerJoinKey);
	if (found != std::string::npos)
	{
		std::size_t foundHash = line.find_last_of('#');
		if (foundHash != std::string::npos)
		{
			if (currentPlayers.empty())
			{
				currentPlayers.insert(localSteamID);
				if (withCallback)
				{
					playerCallback(localSteamID);
					debug() << "Emitting local player " << localSteamID;
				}
			}

			sf::Uint64 steamID = cStoUL(line.substr(foundHash + 1));
			if (steamID != 0 && currentPlayers.count(steamID) == 0)
			{
				currentPlayers.insert(steamID);
				foundMatch = true;
				if (withCallback)
				{
					playerCallback(steamID);
					debug() << "Found player " << steamID;
				}
			}
			else
			{
				//debug() << "Ignoring player " << steamID;
			}
		}
	}

	if (line.find(localPlayerJoinKey) != std::string::npos)
	{
		foundMatch = true;
		expectRating = true;
		if (currentPlayers.empty())
		{
			currentPlayers.insert(localSteamID);
			if (withCallback)
			{
				playerCallback(localSteamID);
				debug() << "Emitting local player " << localSteamID;
			}
		}
	}

	if (line.find(gameEndKey) != std::string::npos)
	{
		resetPlayers();
	}

	found = line.find(rankingScoreKey);
	if (found != std::string::npos)
	{
		double rating = cStoD(line.substr(found + rankingScoreKey.size()));

		if (expectRating)
		{
			debug() << "Found rating " << rating;
			expectRating = false;

			ratingHistory.push_back(std::floor(rating * 10.0 + 0.5));

			if (withCallback && ratingHistory.size() > 1)
			{
				ratingCallback(ratingHistory.back() - ratingHistory[ratingHistory.size() - 2]);
			}

			while (ratingHistory.size() > ratingHistoryLimit)
			{
				ratingHistory.erase(ratingHistory.begin());
			}
		}
		else
		{
			debug() << "Found rating " << rating << " outside of match, skipping";
		}
	}
}

std::vector<std::string> PersistentLogReader::getFileNameList() const
{
	return {"ApplicationPersistent.log"};
}

bool PersistentLogReader::onInitRead()
{
	if (matchStartPos == 0)
	{
		debug() << "No last match start known; searching...";
		bool success = false;
		for (std::size_t i = 1; i < 100; ++i)
		{
			if (seekFromEnd(i * 1000000))
			{
				debug() << "Trying offset " << tell();
				if (tryFindLatestMatch())
				{
					debug() << "Found latest match around " << matchStartPos;
					success = true;
					break;
				}
			}
			else
			{
				debug() << "Seek failed";
				break;
			}
		}

		if (!success)
		{
			seek(0);
			debug() << "Searching from beginning...";
			if (!tryFindLatestMatch())
			{
				debug() << "No matches found in persistent log.";
			}
		}
		requestingStateSave = true;
		return false;
	}
	else
	{
		debug() << "Skipping to last known match start at " << matchStartPos;
		skipToMatchStart();
		return true;
	}
}

void PersistentLogReader::setPlayerCallback(PersistentLogReader::Callback callback)
{
	playerCallback = callback;
}

PersistentLogReader::Callback PersistentLogReader::getPlayerCallback() const
{
	return playerCallback;
}

const std::set<sf::Uint64>& PersistentLogReader::getCurrentPlayers() const
{
	return currentPlayers;
}

void PersistentLogReader::onInitConfig(const cfg::Config& config)
{
	matchStartPos = config.get(cfgMatchStartPos);
	ratingHistory = config.get(cfgRatingHistory);
	ratingHistoryLimit = config.get(cfgRatingHistoryBufferSize);

	if (ratingHistoryLimit < 1)
	{
		ratingHistoryLimit = 50;
	}
}

void PersistentLogReader::saveState(cfg::Config& config) const
{
	config.set(cfgMatchStartPos, matchStartPos);
	config.set(cfgRatingHistory, ratingHistory);
}

void PersistentLogReader::resetPlayers()
{
	previousPlayers = currentPlayers;
	currentPlayers.clear();
}

const std::set<sf::Uint64>& PersistentLogReader::getPreviousPlayers() const
{
	return previousPlayers;
}

void PersistentLogReader::setLocalSteamID(sf::Uint64 steamID)
{
	localSteamID = steamID;
}

bool PersistentLogReader::tryFindLatestMatch()
{
	foundMatch = false;
	expectRating = false;
	resetPlayers();
	ratingHistory.clear();
	std::size_t pos = tell();
	if (readInitial() && foundMatch)
	{
		matchStartPos = pos;
		return true;
	}
	else
	{
		return false;
	}
}

void PersistentLogReader::setRatingCallback(RatingCallback callback)
{
	ratingCallback = callback;
}

PersistentLogReader::RatingCallback PersistentLogReader::getRatingCallback() const
{
	return ratingCallback;
}

sf::Int64 PersistentLogReader::getCurrentRating() const
{
	return ratingHistory.empty() ? 0 : ratingHistory.back();
}

bool PersistentLogReader::checkStateSaveRequest()
{
	if (requestingStateSave)
	{
		requestingStateSave = false;
		return true;
	}
	else
	{
		return false;
	}
}

void PersistentLogReader::emitCurrentData()
{
	debug() << "Queueing " << currentPlayers.size() << " existing players";
	for (const auto & player : currentPlayers)
	{
		playerCallback(player);
	}

	for (std::size_t i = 1; i < ratingHistory.size(); ++i)
	{
		ratingCallback(ratingHistory[i] - ratingHistory[i - 1]);
	}
}
