#ifndef SRC_CLIENT_RANKCHECK_PERSISTENTLOGREADER_HPP_
#define SRC_CLIENT_RANKCHECK_PERSISTENTLOGREADER_HPP_

#include <Client/RankCheck/LogReader.hpp>
#include <SFML/Config.hpp>
#include <cstddef>
#include <functional>
#include <set>
#include <string>
#include <vector>

/**
 * Live-reader for "ApplicationPersistent.log".
 *
 * Detects SteamIDs of players in the match and local rating changes.
 *
 * To avoid parsing the entire file everytime RankCheck starts, the file read position of the current match is saved in
 * RankCheck's config file.
 */
class PersistentLogReader : public LogReader
{
public:

	using Callback = std::function<void(sf::Uint64)>;
	using RatingCallback = std::function<void(sf::Int64)>;

	PersistentLogReader();
	virtual ~PersistentLogReader();

	void resetPlayers();

	void markMatchStart();
	void skipToMatchStart();

	void saveState(cfg::Config & config) const;

	void setPlayerCallback(Callback callback);
	Callback getPlayerCallback() const;

	void setRatingCallback(RatingCallback callback);
	RatingCallback getRatingCallback() const;

	sf::Int64 getCurrentRating() const;

	const std::set<sf::Uint64> & getCurrentPlayers() const;
	const std::set<sf::Uint64> & getPreviousPlayers() const;

	void setLocalSteamID(sf::Uint64 steamID);

	bool checkStateSaveRequest();

private:

	virtual void onReopenFile() override;
	virtual void onReopenFileSuccess() override;
	virtual void onProcessLine(const std::string & line, bool withCallback) override;
	virtual std::vector<std::string> getFileNameList() const override;
	virtual bool onInitRead() override;
	virtual void onInitConfig(const cfg::Config & config) override;

	bool tryFindLatestMatch();
	void emitCurrentData();

	sf::Uint64 localSteamID = 0;
	std::size_t matchStartPos = 0;
	bool foundMatch = false;
	bool expectRating = false;
	bool requestingStateSave = false;
	std::set<sf::Uint64> currentPlayers;
	std::set<sf::Uint64> previousPlayers;
	std::vector<sf::Int64> ratingHistory;
	Callback playerCallback;
	RatingCallback ratingCallback;
	std::size_t ratingHistoryLimit;
};

#endif
