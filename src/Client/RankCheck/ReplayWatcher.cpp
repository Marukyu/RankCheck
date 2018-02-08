#include <Client/RankCheck/PlayerData.hpp>
#include <Client/RankCheck/ReplayWatcher.hpp>
#include <Shared/Config/CompositeTypes.hpp>
#include <Shared/Config/Config.hpp>
#include <Shared/Utils/DebugLog.hpp>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

ReplayWatcher::ReplayWatcher()
{
	callback = [](ReplayParser::ReplayInfo){};
	callbackStart = [](std::string){};
}

ReplayWatcher::~ReplayWatcher()
{
}

void ReplayWatcher::initWithConfig(const cfg::Config& config)
{
	static cfg::String gameFolder("rankcheck.awesomenauts.gameFolder");

	replayFolderObserver.setEventMask(fs::DirectoryObserver::Event::Added);
	replayFolderObserver.setWatchedDirectory(config.get(gameFolder) + "/Data/Replays");
	replayFolderObserver.startWatching();
}

void ReplayWatcher::setCallback(Callback callback)
{
	this->callback = callback;
}

void ReplayWatcher::setReplayStartCallback(CallbackStart callback)
{
	callbackStart = callback;
}

void ReplayWatcher::process()
{
	bool foundReplay = false;

	fs::DirectoryObserver::Event dirEvent;
	while (replayFolderObserver.pollEvent(dirEvent))
	{
		reportedPlayers.clear();
		replayFileObserver.setTargetFile(dirEvent.filename + "/Replays.info");
		debug() << "Found new replay " << replayFileObserver.getTargetFile() << "!";
		callbackStart(dirEvent.filename);
		foundReplay = true;
	}

	if (replayFileObserver.poll() || foundReplay)
	{
		debug() << "Parsing replay " << replayFileObserver.getTargetFile() << "...";
		ReplayParser parser(replayFileObserver.getTargetFile());
		ReplayParser::ReplayInfo info = parser.parse();

		if (info.localTeam != PlayerData::UnknownTeam)
		{
			for (auto it = info.players.begin(); it != info.players.end(); )
			{
				if (reportedPlayers.count(it->player.steamID))
				{
					it = info.players.erase(it);
				}
				else
				{
					reportedPlayers.insert(it->player.steamID);
					++it;
				}
			}

			if (!info.players.empty())
			{
				callback(info);
			}
		}
	}
}
