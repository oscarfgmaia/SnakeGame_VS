#include <algorithm>
#include <chrono>
//#include "list.hpp"
#include <cmath>
#include <deque>
#include "raylib.h"

using Body = std::deque<Vector2>;
constexpr int kDimensionSize = 32;
const int kScreenWidth = 800;
const int kScreenHeight = 448;
int is_game_paused = 0;
using GameScreen = enum GameScreen { kTitle = 0, kGameplay, kEnding, kPause};
GameScreen current_screen;

bool operator==(const Vector2 lhs, const Vector2 rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y;
}
class Food {
public:
	Vector2 m_position;
	Rectangle sprite{ 0, 0, kDimensionSize, kDimensionSize };
	void SetNewPosition() {
		auto start = std::chrono::system_clock::now().time_since_epoch().count();
		Vector2 pos = { static_cast<float>(GetRandomValue(0, 24)),
					   static_cast<float>(GetRandomValue(0, 13)) };
		SetRandomSeed(start);
		m_position = pos;
	}
	void Draw() {
		sprite.x = m_position.x * kDimensionSize;
		sprite.y = m_position.y * kDimensionSize;
		DrawRectangleRec(sprite, GREEN);
	}
};

template <typename T>
constexpr float GetTurnTime(T size) {
	constexpr float kInitialTurnTime = 1.f / 10.f;
	constexpr float kBaseTime = 1.f / 60.f;
	float interp = std::lerp(0.f, 2.f, static_cast<float>(size - 1) / 100.f);
	float turn_time = ((kInitialTurnTime - kBaseTime) * std::expf(-interp)) + kBaseTime;
	return turn_time;
}

class Player {
public:
	enum class Direction { kUp, kDown, kLeft, kRight };
	Direction direction = Direction::kRight;
	Body body;
	Player() {
		body.push_front(Vector2{ 3, 6 });
		body.push_back(Vector2{ 2, 6 });
		body.push_back(Vector2{ 1, 6 });
	}

	Vector2& Position() { return body.front(); }

	int Size() const { return body.size(); }

	bool HeadIntersectsWithBody() {
		// ...
		auto head = body.front();
		for (size_t i = 1; i < body.size(); ++i) {
			if (head == body[i]) {
				return true;
			}
		}
		return false;
	}

	Vector2 NextPosition() {
		auto pos = Position();

		switch (direction) {
		case Direction::kUp: {
			pos.y -= 1;
			break;
		}
		case Direction::kDown: {
			pos.y += 1;
			break;
		}
		case Direction::kLeft: {
			pos.x -= 1;
			break;
		}
		case Direction::kRight: {
			pos.x += 1;
			break;
		}
		}
		return pos;
	}

	void Move() {
		body.pop_back();
		body.push_front(NextPosition());
	}
};

void RenderPlayer(Player& p) {
	auto draw_node = [](Vector2 pos) {
		auto& [x, y] = pos;
		auto r = Rectangle{ x * kDimensionSize, y * kDimensionSize, kDimensionSize, kDimensionSize };
		DrawRectangleRec(r, BLACK);
	};

	for (const auto& node : p.body) {
		draw_node(node);
	}
}
// S() : n(7) {} // constructor definition:
class Game {
public:
	Food food;
	Player player;

	int score = 0;
	float timer = 1;
	float velocity = 0.1;
	enum class States { kPlaying, kGameover, kPause };
	States state;

	void ResetPlayer() { player = Player{}; }

	void ResetFood() {
		food = Food{};
		food.SetNewPosition();
	}

	bool FoodIntersectsWithPlayerBody() {
		// ...
		for (size_t i = 1; i < player.body.size(); ++i) {
			if (food.m_position.x == player.body[i].x && food.m_position.y == player.body[i].y) {
				return true;
			}
		}
		return false;
	}
	bool CheckCollisionWithWall() {
		if (player.Position().x >= kScreenWidth / 32.0f || player.Position().x < 0 ||
			player.Position().y < 0 || player.Position().y >= kScreenHeight / 32.0f) {
			state = States::kGameover;
			return true;
		}
		state = States::kPlaying;
		return false;
	}

	void ChangeFoodLocation() {
		food.SetNewPosition();
		if (FoodIntersectsWithPlayerBody()) {
			food.SetNewPosition();
			++score;
		}
	}

	void ContinueGame() {
		is_game_paused = 0;
		state = States::kPlaying;
	}
	void PauseGame() {
		is_game_paused = 1;
		state = States::kPause;
	}

	void IncreaseBodySize() {
		player.body.push_front(player.NextPosition());
		score++;
	}

	bool CheckCollisionWithFood() { return player.NextPosition() == food.m_position; }

	bool CheckCollisionWithBody() { return player.HeadIntersectsWithBody(); }

	void GameOver() {
		state = States::kGameover;
		current_screen = kEnding;
		score = 0;
	}
	void GameInit() { food.SetNewPosition(); }

	bool NextStep() {
		timer -= GetFrameTime();
		if (timer > 0) {
			return false;
		}
		timer = GetTurnTime(player.Size());
		return true;
	}

	void GameDrawMessages() const {
		switch (state) {
		case States::kPlaying: {
			DrawText("PLAYING", 190, 200, 20, LIME);
			break;
		}
		case States::kGameover: {
			DrawText("GAME OVER", 190, 200, 20, RED);
			break;
		}
		case States::kPause: {
			DrawText("GAME PAUSED", 190, 200, 20, RED);
			break;
		}
		default: {
			break;
		}
		}
	}

	void GameUpdate() {
		if ((IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) && player.direction != Player::Direction::kLeft) {
			player.direction = Player::Direction::kRight;
		}
		if ((IsKeyPressed(KEY_LEFT) ||
			IsKeyPressed(KEY_A)) && player.direction != Player::Direction::kRight) {
			player.direction = Player::Direction::kLeft;
		}
		if ((IsKeyPressed(KEY_UP) ||
			IsKeyPressed(KEY_W)) && player.direction != Player::Direction::kDown) {
			player.direction = Player::Direction::kUp;
		}
		if ((IsKeyPressed(KEY_DOWN) ||
			IsKeyPressed(KEY_S)) && player.direction != Player::Direction::kUp) {
			player.direction = Player::Direction::kDown;
		}
		if (IsKeyPressed(KEY_P)) {
			if (is_game_paused == 1) {

				ContinueGame();
			}
			else {
				PauseGame();
			}
		}

		if (NextStep()) {
			if (is_game_paused == 0) {
				if (CheckCollisionWithFood()) {
					ChangeFoodLocation();
					IncreaseBodySize();
				}
				else {
					player.Move();
				}

				if (CheckCollisionWithBody()) {
					GameOver();
				}
				else {
				}
				if (CheckCollisionWithWall()) {
					GameOver();
				}
			}
		}
	}

	void DrawPlaying() {
		RenderPlayer(player);
		food.Draw();
		// TODO(oscar): TIRAR ESSE MOI DE LIXO
		DrawText(TextFormat("PRESS P TO PAUSE"), 10, kScreenHeight-20, 20, LIGHTGRAY);
		DrawText(TextFormat("SCORE: %1i", score), kScreenWidth-140, kScreenHeight - 20, 20, BLUE);
		//DrawText(TextFormat("X: %2.0f", food.m_position.x), 200, 360, 20, RED);
		//DrawText(TextFormat("Y: %2.0f", food.m_position.y), 300, 360, 20, RED);
		// DrawText(TextFormat("X: %2.0f", player.Position()), 200, 400, 20, BLACK);
		// DrawText(TextFormat("Y: %2.0f", player.Position().y), 300, 400, 20, BLACK);
		// GameDrawMessages();
	}
};

int main() {
	// Initialization
	//--------------------------------------------------------------------------------------
	Game game;
	game.GameInit();
	current_screen = kTitle;
	InitWindow(kScreenWidth, kScreenHeight, "Snake Game");
	// SetTargetFPS(144);  // Set our game to run at 144 frames-per-second
	//--------------------------------------------------------------------------------------
	SetWindowState(FLAG_VSYNC_HINT);

	// Main game loop
	// Update
	while (!WindowShouldClose())  // Detect window close button or ESC key
	{
		switch (current_screen) {
		case kTitle: {
			// TODO(oscar): Update TITLE screen variables here!
			if (IsKeyPressed(KEY_ENTER)) {
				current_screen = kGameplay;
			}

		} break;
		case kGameplay: {
			// TODO(oscar): Update GAMEPLAY screen variables here!

			game.GameUpdate();

		} break;
		case kEnding: {
			// TODO(oscar): Update ENDING screen variables here!
			if (IsKeyPressed(KEY_ENTER)) {
				game.ResetFood();
				game.ResetPlayer();
				current_screen = kGameplay;
			}

		} break;
		case kPause: {
			// TODO(oscar): Update TITLE screen variables here!
			if (IsKeyPressed(KEY_P)) {
				is_game_paused == 0;
				current_screen = kGameplay;
			}

		} break;
		default:
			break;
		}

		// game.GameUpdate();

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();
		DrawFPS(10, 10);
		// game.Draw();
		ClearBackground(RAYWHITE);
		switch (current_screen) {
		case kTitle: {
			// TODO(oscar): Draw TITLE screen here!n here!
			DrawText("TITLE SCREEN", 20, 20, 40, DARKGREEN);
			DrawText("PRESS ENTER or TAP to JUMP to GAMEPLAY SCREEN", 120, 220, 20, DARKGREEN);

		} break;
		case kGameplay: {
			// TODO(oscar): Draw GAMEPLAY screen here!
			game.DrawPlaying();
			//DrawText("GAMEPLAY SCREEN", 20, 20, 40, MAROON);

		} break;
		case kEnding: {
			// TODO(oscar): Draw ENDING screen here!

			DrawText("ENDING SCREEN", 20, 20, 40, DARKBLUE);
			DrawText("PRESS ENTER or TAP to RETURN to TITLE SCREEN", 120, 220, 20, DARKBLUE);
		} break;
		case kPause: {
			// TODO(oscar): Draw ENDING screen here!

			DrawText("PAUSE GAME", 20, 20, 40, DARKBLUE);
			DrawText("PRESS P TO RETURN TO GAMEPLAY", 120, 220, 20, DARKBLUE);
		} break;
		default:
			break;
		}
		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// De-Initialization
	//--------------------------------------------------------------------------------------
	CloseWindow();  // Close window and OpenGL context
	//--------------------------------------------------------------------------------------

	return 0;
}