#ifndef SRC_CLIENT_RANKCHECK_REPLAYWATCHER_HPP_
#define SRC_CLIENT_RANKCHECK_REPLAYWATCHER_HPP_

#include <Client/RankCheck/ReplayParser.hpp>
#include <SFML/Config.hpp>
#include <Shared/Utils/Filesystem/DirectoryObserver.hpp>
#include <Shared/Utils/Filesystem/FileObserver.hpp>
#include <functional>
#include <set>

namespace cfg
{
class Config;
}

class ReplayWatcher
{
public:
	ReplayWatcher();
	~ReplayWatcher();

	using Callback = std::function<void(ReplayParser::ReplayInfo)>;
	using CallbackStart = std::function<void(std::string)>;

	void initWithConfig(const cfg::Config & config);
	void setCallback(Callback callback);
	void setReplayStartCallback(CallbackStart callback);
	void process();

private:

	Callback callback;
	CallbackStart callbackStart;
	fs::DirectoryObserver replayFolderObserver;
	fs::FileObserver replayFileObserver;

	std::set<sf::Uint64> reportedPlayers;
};

#endif
