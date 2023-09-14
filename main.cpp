#include <array>
#include <chrono>
#include <ctime>
#include <SFML/Graphics.hpp>

#include "Global.hpp"
#include "DrawText.hpp"
#include "Pacman.hpp"
#include "Ghost.hpp"
#include "GhostManager.hpp"
#include "ConvertSketch.hpp"
#include "DrawMap.hpp"
#include "MapCollision.hpp"


int main()
{
	//Is the game won?
	bool game_won = 0;

	//Used to make the game framerate-independent.
	unsigned lag = 0;

	unsigned char level = 0;

	//Similar to lag, used to make the game framerate-independent.
	std::chrono::time_point<std::chrono::steady_clock> previous_time;

	//It's not exactly like the map from the original Pac-Man game, but it's close enough.
	/*std::array<std::string, MAP_HEIGHT> map_sketch1 = {
		" ################### ",
		" #........#........# ",
		" #o##.###.#.###.##o# ",
		" #.................# ",
		" #.##.#.#####.#.##.# ",
		" #....#...#...#....# ",
		" ####.### # ###.#### ",
		"    #.#   0   #.#    ",
		"#####.# ##=## #.#####",
		"     .  #123#  .     ",
		"#####.# ##### #.#####",
		"    #.#       #.#    ",
		" ####.# ##### #.#### ",
		" #........#........# ",
		" #.##.###.#.###.##.# ",
		" #o.#.....P.....#.o# ",
		" ##.#.#.#####.#.#.## ",
		" #....#...#...#....# ",
		" #.######.#.######.# ",
		" #.................# ",
		" ################### "
	};*/

	/*std::array<std::string, MAP_HEIGHT> map_sketch2 = {
		" ################### ",
		" #........#........# ",
		" #.##.###.#.###.##.# ",
		" #.................# ",
		" #.##.#.#####.#.##.# ",
		" #....#...#...#....# ",
		" ####.### # ###.#### ",
		"    #.#   0   #.#    ",
		"#####.# ##=## #.#####",
		"     .  #123#  o     ",
		"#####.# ##### #.#####",
		"    #.#       #.#    ",
		" ####.# ##### #.#### ",
		" #........#........# ",
		" #.##.###.#.###.##.# ",
		" #o.#.....P.....#.o# ",
		" ##.#.#.#####.#.#.## ",
		" #....#...#...#....# ",
		" #.######.#.######.# ",
		" #........o........# ",
		" ################### "
	};*/

	std::array<std::string, MAP_HEIGHT> map_sketch3 = {
		" ##################### ",
		" #o...#........o#....# ",
		" #.##.#.#######.#.##.# ",
		" #.#.......0.......#.# ",
		" #.#.##.###=###.##.#.# ",
		" #.#....# 123 #....#.# ",
		" #...##.#######.##...# ",
		" #.#.......P.......#.# ",
		" #.#.##.#######.##.#.# ",
		" #.#...............#.# ",
		" #.##.#.#######.#.##.# ",
		" #...o#.........#...o# ",
		" ##################### "
	};

	std::array<std::array<Cell, MAP_HEIGHT>, MAP_WIDTH> map{};

	//Initial ghost positions.
	std::array<Position, 4 > ghost_positions;

	sf::Event event;

	sf::RenderWindow window(sf::VideoMode(2*CELL_SIZE * MAP_WIDTH * SCREEN_RESIZE, 2*(FONT_HEIGHT + CELL_SIZE * MAP_HEIGHT) * SCREEN_RESIZE), "Super-Pacman!!",sf::Style::Close);
	window.setFramerateLimit(240);
	//Resizing the window.
	window.setView(sf::View(sf::FloatRect(0, 0, CELL_SIZE * MAP_WIDTH, FONT_HEIGHT + CELL_SIZE * MAP_HEIGHT)));

	GhostManager ghost_manager;

	Pacman pacman;

	//Generating a random seed.
	srand(static_cast<unsigned>(time(0)));

	map = convert_sketch(map_sketch3, ghost_positions, pacman);

	ghost_manager.reset(level, ghost_positions);

	//Get the current time and store it in a variable.
	previous_time = std::chrono::steady_clock::now();

	while (1 == window.isOpen())
	{
		//Here we're calculating the lag.
		unsigned delta_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - previous_time).count();

		lag += delta_time;

		previous_time += std::chrono::microseconds(delta_time);

		//While the lag exceeds the maximum allowed frame duration.
		while (FRAME_DURATION <= lag)
		{
			//We decrease the lag.
			lag -= FRAME_DURATION;

			while (1 == window.pollEvent(event))
			{
				switch (event.type)
				{
				case sf::Event::Closed:
				{
					//Making sure the player can close the window.
					window.close();
				}
				}
			}

			if (0 == game_won && 0 == pacman.get_dead())
			{
				game_won = 1;

				pacman.update(level, map);

				ghost_manager.update(level, map, pacman);

				//We're checking every cell in the map.
				for (const std::array<Cell, MAP_HEIGHT>& column : map)
				{
					for (const Cell& cell : column)
					{
						if (Cell::Pellet == cell) //And if at least one of them has a pellet.
						{
							game_won = 0; //The game is not yet won.

							break;
						}
					}

					if (0 == game_won)
					{
						break;
					}
				}

				if (1 == game_won)
				{
					pacman.set_animation_timer(0);
				}
			}
			else if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) //Restarting the game.
			{
				game_won = 0;

				if (1 == pacman.get_dead())
				{
					level = 0;
				}
				else
				{
					//After each win we reduce the duration of attack waves and energizers.
					level++;
				}
				
				if (level == 0) {
					map = convert_sketch(map_sketch3, ghost_positions, pacman);
				}

				else if (level == 1) {
					map = convert_sketch(map_sketch3, ghost_positions, pacman);
				}
				//map = convert_sketch(map_sketch1, ghost_positions, pacman);

				ghost_manager.reset(level, ghost_positions);

				pacman.reset();
			}

			if (1 == sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
				window.close();
			}
			//I don't think anything needs to be explained here.
			if (FRAME_DURATION > lag)
			{
				window.clear();

				if (0 == game_won && 0 == pacman.get_dead())
				{
					draw_map(map, window);

					ghost_manager.draw(GHOST_FLASH_START >= pacman.get_energizer_timer(), window);

					draw_text(0, 0, CELL_SIZE * MAP_HEIGHT, "Level: " + std::to_string(1 + level), window);
					draw_text(0, 170, CELL_SIZE * MAP_HEIGHT, "Pearapat Kumsing 66010587", window);
				}

				pacman.draw(game_won, window);

				if (1 == pacman.get_animation_over())
				{
					if (1 == game_won)
					{
						draw_text(1, 0, 0, "Next level!!", window);
					}
					else
					{
						draw_text(1, 0, 0, "Game over\nT_T", window);
					}
				}

				window.display();
			}
		}
	}
}
