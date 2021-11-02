#include <sstream>
#include <uWebSockets/App.h>
#include <unordered_set>

struct string_equal {
	using is_transparent = std::true_type;

	bool operator()(std::string_view l, std::string_view r) const noexcept {
		return l == r;
	}
};
struct string_hash {
	using is_transparent = std::true_type;

	auto operator()(std::string_view str) const noexcept {
		return std::hash<std::string_view>()(str);
	}
};

struct UserData {
	std::string name;
};
std::unordered_set<std::string, string_hash, string_equal> usernames;
size_t user_idx = 0;

std::tuple<std::string_view, std::string_view> get_command(std::string_view const message) {
	auto const space_pos = message.find(' ');
	if (space_pos == std::string_view::npos) {
		return { message, "" };
	} else {
		return { message.substr(0, space_pos), message.substr(space_pos + 1) };
	}
}

auto make_ws_struct() {
	uWS::App::WebSocketBehavior<UserData> ret;
	ret.open =
		[](auto* ws) {
			std::ostringstream greeting;
			greeting << "Welcome to chat. Your name is " << user_idx << ". Change it with `/name newname`.";
			ws->getUserData()->name = std::to_string(user_idx);
			usernames.emplace(std::as_const(ws->getUserData()->name));
			user_idx++;
			ws->send(greeting.str(), uWS::OpCode::TEXT);
			ws->subscribe("chat");
		},
	ret.message =
		[](auto* ws, std::string_view message, uWS::OpCode) {
			if (message.starts_with('/')) {
				message.remove_prefix(1);
				auto [command, args] = get_command(message);
				if (command == "name") {
					if (args == ws->getUserData()->name) {
						return;
					}
					if (usernames.find(args) != usernames.end()) {
						ws->send("Username taken", uWS::OpCode::TEXT);
						return;
					}
					usernames.erase(ws->getUserData()->name);
					ws->getUserData()->name = args;
					usernames.emplace(std::as_const(ws->getUserData()->name));
					ws->send("Your name has been changed", uWS::OpCode::TEXT);
				} else {
					std::ostringstream output;
					output << "Unknown command /" << command;
					ws->send(output.str(), uWS::OpCode::TEXT);
				}
			} else {
				std::ostringstream output;
				output << '[' << ws->getUserData()->name << "] " << message;
				ws->publish("chat", output.str(), uWS::OpCode::TEXT);
			}
		},
	ret.close = [](auto*, int, std::string_view) {
		//
	};
	return ret;
}

int main() {
	uWS::App()
		.ws<UserData>("/chat", make_ws_struct())
		.listen(
			9001,
			[](auto*) {
				std::clog << "Listening on port 9001" << std::endl;
			})
		.run();
	return 0;
}
