#define _USE_MATH_DEFINES
#define _POSIX_SOURCE
#include<math.h>
#include<stdio.h>
#include<stdlib.h> 
#include<string.h>
#include<time.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define CarWidth 34
#define CarHeight 57
#define StoneWidth 19
#define StoneHeight 39
#define PuddleWidth 25
#define PuddleHeight 25
#define BulletWidth 2
#define BulletHeight 6
#define PlayerSpeedY 100
#define PlayerSpeedX 1
#define PlayerMaxSpeed 300
#define PlayerMinSpeed 100
#define CAR 'c'
#define TREE 't'
#define BULLET 'b'
#define PUDDLE 'p'
#define STONE 's'
#define TxtMaxARR 100
#define GrassWidth 150
#define SpacesBetweenLines 10
#define MaxSaves 5
#define PENALTY 3
#define RESETDISTANCE 2000

struct Object
{
	int iy;
	int x;
	int y;
	char c;
	bool IsAlive;
};

struct velocity
{
	int x;
	int y;
};

struct Data
{
	int velocity;
	int distance;
	double worldtime;
	float points;
};

// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) 
{
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};

// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) 
{
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};

// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) 
{
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};

// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) 
{
	for(int i = 0; i < l; i++) 
	{
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};

// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor) 
{
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

void PausedGame(bool& pause, int& t1, int& t2, double& delta, SDL_Surface* screen,
	SDL_Surface* Pause, SDL_Texture* scrtex, SDL_Renderer* renderer,
	velocity& v, Data& Save, double& distance, float& points,
	double& worldTime)
{
	if (pause == true) 
	{
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;
		DrawSurface(screen, Pause, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_Event event;
		while (SDL_PollEvent(&event)) 
		{
			switch (event.type) {
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) 
				{
				case SDLK_p:
					pause = false;
					v.y = Save.velocity;
					worldTime = Save.worldtime - delta;
					points = Save.points;
					distance = Save.distance;
					break;
				}
				break;
			}
		}
	}
}

void PutObstacles(Object obstacle[], const int number, const char c)
{
	int min = -RESETDISTANCE;
	const int max = 0;
	const int MIN = 170;
	const int MAX = 470;  
	for (int i = 0; i < number; i++)
	{
		if (c == 'c')
			min = -RESETDISTANCE/2;
		if (c == 't')
		{
			if (i % 2 == 0)
			{
				obstacle[i].x = rand() % (GrassWidth - CarWidth/2) + max;
				obstacle[i].iy = rand() % (max - min + 1) + min;
				obstacle[i].c = c;
				obstacle[i].IsAlive = true;
			}
			else
			{
				obstacle[i].x = rand() % (GrassWidth - CarWidth / 2) + (SCREEN_WIDTH - GrassWidth + CarWidth/2);
				obstacle[i].iy = rand() % (max - min + 1) + min;
				obstacle[i].c = c;
				obstacle[i].IsAlive = true;
			}
		}
		else
		{
			obstacle[i].x = rand() % (MAX - MIN + 1) + MIN;
			obstacle[i].iy = rand() % (max - min + 1) + min;
			obstacle[i].c = c;
			obstacle[i].IsAlive = true;
		}
	}
}

int Width(Object o)
{
	if (o.c == STONE)
		return StoneWidth;
	else if (o.c == CAR)
		return CarWidth;
	else if (o.c == PUDDLE)
		return PuddleWidth;
	else if (o.c == BULLET)
		return BulletWidth;
}

int Height(Object o)
{
	if (o.c == STONE)
		return StoneHeight;
	else if (o.c == CAR)
		return CarHeight;
	else if (o.c == PUDDLE)
		return PuddleHeight;
	else if (o.c == BULLET)
		return BulletHeight;
}

bool CheckCollision(Object o1, Object o2)
{
	if (o1.x < o2.x + Width(o2) && o1.x + Width(o1) > o2.x &&
		o1.y < o2.y + Height(o2) && o1.y + Height(o1) > o2.y)
		return true;
	return false;
}

bool CheckAllCollisions(Object car, Object c[], Object s[], Object p[], const int nc, const int ns, const int np)
{
	for (int i = 0; i < nc; i++)
		if (CheckCollision(car, c[i]) == true)
			return true;
	for (int i = 0; i < ns; i++)
		if (CheckCollision(car, s[i]) == true)
			return true;
	for (int i = 0; i < np; i++)
		if (CheckCollision(car, p[i]) == true)
			return true;
}

void CheckKill(Object c[], Object b[], int nc, int nb, bool& kill)
{
	for (int i = 0; i < nc; i++)
		for (int j = 0; j < nb; j++) 
		{
			if (CheckCollision(b[j], c[i]) == true)
			{
				c[i].y = NULL;
				c[i].x = NULL;
				b[i].x = NULL;
				b[i].y = NULL;
				c[i].IsAlive = false;
				b[i].IsAlive = false;
				kill = true;
			}
		}
}

void LoadGame(double& distance, Object s[], int& ns, Object c[], int& nc, Object p[], int& np,
	Object t[], int& nt, Object b[], int& number, Object& car, float& points, double& worldTime, velocity& v, char fileName[])
{
	char txt[TxtMaxARR];
	sprintf(txt, "%s.dat", fileName);
	FILE* fp = fopen(txt, "r");
	if (fp == NULL) 
	{
		printf("Error: Could not open file %s\n", fileName);
		return;
	}
	fscanf(fp, "Distance: %lf\n", &distance);
	fscanf(fp, "Points: %f\n", &points);
	fscanf(fp, "WorldTime: %lf\n", &worldTime);
	fscanf(fp, "Velocity: %d %d\n", &v.x, &v.y);
	fscanf(fp, "Car: %d %d %d %c\n", &car.x, &car.y, &car.IsAlive, &car.c);
	fscanf(fp, "Stones: %d\n", &ns);
	for (int i = 0; i < ns; i++) 
		fscanf(fp, "%d %d %d %c\n", &s[i].x, &s[i].iy, &s[i].IsAlive, &s[i].c);
	fscanf(fp, "Cars: %d\n", &nc);
	for (int i = 0; i < nc; i++) 
		fscanf(fp, "%d %d %d %c\n", &t[i].x, &t[i].iy, &t[i].IsAlive, &t[i].c);
	fscanf(fp, "Puddles: %d\n", &np);
	for (int i = 0; i < np; i++) 
		fscanf(fp, "%d %d %d %c\n", &p[i].x, &p[i].iy, &p[i].IsAlive, &p[i].c);
	fscanf(fp, "Trees: %d\n", &nt);
	for (int i = 0; i < nt; i++) 
		fscanf(fp, "%d %d %d %c\n", &t[i].x, &t[i].iy, &t[i].IsAlive, &t[i].c);
	fscanf(fp, "Bullets: %d\n", &number);
	for (int i = 0; i < number; i++) 
		fscanf(fp, "%d %d %d %c\n", &b[i].x, &b[i].iy, &b[i].IsAlive, &b[i].c);
	fclose(fp);
}

void LoadGameInfo2(int lineNum,double& distance, Object s[], int& ns, Object c[], 
	int& nc, Object p[], int& np, Object t[], int& nt, Object b[], int& number, 
	Object& car, float& points, double& worldTime,
	velocity& v)
{
	char fileName[TxtMaxARR];
	FILE* fp = fopen("saves.txt", "r");
	for (int i = 0; i < lineNum; i++) 
		fgets(fileName, sizeof(fileName), fp);
	fclose(fp);
	// remove newline at the end
	fileName[strlen(fileName) - 1] = '\0';
	LoadGame(distance,s,ns,c,nc,p,np,t,nt,b,
	number,car,points,worldTime,v,fileName);
}

void LoadGameInfo(SDL_Surface* screen, SDL_Surface* charset,int czarny, int niebieski, 
	SDL_Texture*& scrtex, SDL_Renderer*& renderer,double& distance, Object s[], int& 
	ns, Object c[], int& nc, Object p[], int& np, Object t[], int& nt, Object b[], 
	int& number, Object& car, float& points, double& worldTime, velocity& v)
{
	int lineNum = 1;
	// Open the saved games file
	FILE* fp = fopen("saves.txt", "r");
	if (fp == NULL)
	{
		printf("Error: Could not open saved_games.txt\n");
		return;
	}
	// Read each line from the file and append it to the text string
	char line[TxtMaxARR];
	SDL_FillRect(screen, NULL, czarny);
	while (fgets(line, sizeof(line), fp) != NULL)
	{
		char text[TxtMaxARR] = "";
		line[strlen(line) - 1] = '\0';
		// Use sprintf to format the line number and line as a single string
		char formattedLine[TxtMaxARR];
		sprintf(formattedLine, "%d. %s", lineNum, line);
		// Append the formatted line to the text string
		strcat(text, formattedLine);
		strcat(text, "\n");
		DrawString(screen, 200, SpacesBetweenLines + lineNum * SpacesBetweenLines, text, charset);
		if (lineNum == MaxSaves)
			break;
		lineNum++;
	}
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
	SDL_PumpEvents();
	bool loading = true;
	SDL_Event event;
	while (loading)
	{
		SDL_WaitEvent(&event);
		switch (event.type)
		{
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
			case SDLK_1:
				LoadGameInfo2(1, distance, s, ns, c, nc, p, np, t, nt, b, 
				number, car, points, worldTime, v);
				loading = false;
				break;
			case SDLK_2:
				LoadGameInfo2(2, distance, s, ns, c, nc, p, np, t, nt, b, 
				number, car, points, worldTime, v);
				loading = false;
				break;
			case SDLK_3:
				LoadGameInfo2(3, distance, s, ns, c, nc, p, np, t, nt, b,
				number, car, points, worldTime, v);
				loading = false;
				break;
			case SDLK_4:
				LoadGameInfo2(4, distance, s, ns, c, nc, p, np, t, nt, b, 
				number, car, points, worldTime, v);
				loading = false;
				break;
			case SDLK_5:
				LoadGameInfo2(5, distance, s, ns, c, nc, p, np, t, nt, b,
				number, car, points, worldTime, v);
				loading = false;
				break;
			}
		}
	}
	fclose(fp);
}

void SaveGameInfo(char* timeString) 
{
	FILE* fp = fopen("saves.txt", "a");
	if (fp != NULL) {
		fprintf(fp, "%s\n", timeString);
		fclose(fp);
	}
}

void replaceSpacesWithSlash(char* str) 
{
	int i = 0;
	while (str[i] != NULL) {
		if (str[i] == ' ')
			str[i] = '-';
		else if (str[i] == ':')
			str[i] = '-';
		i++;
	}
}

void SaveGame(double distance, Object s[], int ns, Object c[], int nc, Object p[],
	int np, Object t[], int nt, Object b[], int number, Object car, double worldTime, 
	velocity v,float points) 
{
	time_t currentTime;
	time(&currentTime); //setting current time
	char* timeString = ctime(&currentTime);
	replaceSpacesWithSlash(timeString); //removing spaces in order to save (error occurs when it is not removed)
	timeString[strlen(timeString) - 1] = '\0'; // remove newline at the end
	char fileName[100];
	sprintf(fileName, "%s.dat", timeString); //adding .dat to the time
	FILE* fp = fopen(fileName, "w");
	SaveGameInfo(timeString);
	//Uploadting data to the file
	fprintf(fp, "Distance: %lf\n", distance);
	fprintf(fp, "Points: %f\n", points);
	fprintf(fp, "WorldTime: %lf\n", worldTime);
	fprintf(fp, "Velocity: %d %d\n", v.x, v.y);
	fprintf(fp, "Car: %d %d %d %c\n", car.x, car.y, car.IsAlive, car.c);
	fprintf(fp, "Stones: %d\n", ns);
	for (int i = 0; i < ns; i++) {
		fprintf(fp, "%d %d %d %c\n", s[i].x, s[i].iy, s[i].IsAlive, s[i].c);
	}
	fprintf(fp, "Cars: %d\n", nc);
	for (int i = 0; i < nc; i++) {
		fprintf(fp, "%d %d %d %c\n", c[i].x, c[i].iy, c[i].IsAlive, c[i].c);
	}
	fprintf(fp, "Puddles: %d\n", np);
	for (int i = 0; i < np; i++) {
		fprintf(fp, "%d %d %d %c\n", p[i].x, p[i].iy, p[i].IsAlive, p[i].c);
	}
	fprintf(fp, "Trees: %d\n", nt);
	for (int i = 0; i < nt; i++) {
		fprintf(fp, "%d %d %d %c\n", t[i].x, t[i].iy, t[i].IsAlive, t[i].c);
	}
	fprintf(fp, "Bullets: %d\n", number);
	for (int i = 0; i < number; i++) {
		fprintf(fp, "%d %d %d %c\n", b[i].x, b[i].iy, b[i].IsAlive, b[i].c);
	}
	fclose(fp);
}

void MainMenu(SDL_Surface*& screen, SDL_Renderer*& renderer, SDL_Surface*& mainmenu, bool& game, 
	bool& menu, int& frames, double& fpsTimer, double& fps, float& points,
	double& worldTime, double& distance, int& number, velocity& v, Object& car, 
	int czarny, SDL_Texture*& scrtex, Object* s, Object* c, Object* p, Object* t, 
	int ns, int nc, int np, int nt, SDL_Surface* charset, Object* b, int niebieski, 
	bool& kill)
{
	SDL_FillRect(screen, NULL, czarny);
	DrawSurface(screen, mainmenu, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
	SDL_PumpEvents();
	SDL_Event event;
	while (menu) 
	{
		SDL_WaitEvent(&event);
		switch (event.type) 
		{
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
			case SDLK_n:
				menu = false;	//Setting strating data for the game
				frames = 0;
				fpsTimer = 0;
				fps = 0;
				points = 0;
				worldTime = 0;
				distance = 0;
				number = 0;
				kill = 0;
				v.y = PlayerSpeedY;
				v.x = 0;
				car.x = SCREEN_WIDTH / 2;
				car.y = SCREEN_HEIGHT / 2;
				car.IsAlive = true;
				car.c = CAR;
				PutObstacles(s, ns, STONE);
				PutObstacles(c, nc, CAR);
				PutObstacles(p, np, PUDDLE);
				PutObstacles(t, nt, TREE);
				game = true;
				break;
			case SDLK_l:
				LoadGameInfo(screen, charset,czarny, niebieski, scrtex, renderer, distance, 
				s, ns, c, nc, p, np, t, nt, b, number, car, points, worldTime, v);
				menu = false;
				game = true;
				break;
			}
		}
	}
}

int LoadBMP(SDL_Surface* &car1, SDL_Surface* &stone,
	SDL_Surface* &mainmenu,
	SDL_Surface* &carr2,SDL_Surface* &paddle,
	SDL_Surface* &tree,SDL_Surface* &Pause,
	SDL_Surface* &bullet ,SDL_Texture* &scrtex,
	SDL_Window* &window, SDL_Renderer* &renderer, 
	SDL_Surface* &screen, SDL_Surface* &charset, int rc)
{
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if (charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};
	SDL_SetColorKey(charset, true, 0x000000);

	car1 = SDL_LoadBMP("./car.bmp");
	if (car1 == NULL) {
		printf("SDL_LoadBMP(car.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	Pause = SDL_LoadBMP("./pause.bmp");
	if (Pause == NULL) {
		printf("SDL_LoadBMP(pause.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	stone = SDL_LoadBMP("./stone.bmp");
	if (stone == NULL) {
		printf("SDL_LoadBMP(stone.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	mainmenu = SDL_LoadBMP("./MainMenu.bmp");
	if (mainmenu == NULL) {
		printf("SDL_LoadBMP(MainMenu.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	carr2 = SDL_LoadBMP("./car2.bmp");
	if (carr2 == NULL) {
		printf("SDL_LoadBMP(car2.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	paddle = SDL_LoadBMP("./Puddle.bmp");
	if (paddle == NULL) {
		printf("SDL_LoadBMP(Puddle.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	tree = SDL_LoadBMP("./Tree.bmp");
	if (tree == NULL) {
		printf("SDL_LoadBMP(Tree.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	bullet = SDL_LoadBMP("./bullet.bmp");
	if (bullet == NULL) {
		printf("SDL_LoadBMP(bullet.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	};

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
		&window, &renderer);
	if (rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	};
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_SetWindowTitle(window, "Aleksander Blok s188865");
	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_ShowCursor(SDL_DISABLE);
}

void DrawObjects(SDL_Surface* screen, Object s[], int ns, Object c[], int nc, Object p[], int np,
	Object t[], int nt, Object b[], int& number, Object& car,
	SDL_Surface* stone, SDL_Surface* paddle, SDL_Surface* carr2,
	SDL_Surface* tree, SDL_Surface* bullet, SDL_Surface* car1, int czarny, 
	int zielony, int czerwony, int niebieski, char text[], SDL_Surface* charset, 
	double& worldTime, float& points)
{
	SDL_FillRect(screen, NULL, czarny);
	DrawRectangle(screen, 0, 0, GrassWidth, SCREEN_HEIGHT, zielony, zielony);
	DrawRectangle(screen, SCREEN_WIDTH - GrassWidth, 0, GrassWidth, SCREEN_HEIGHT, zielony, zielony);
	for (int i = 0; i < ns; i++)
		if (s[i].IsAlive == true)
			DrawSurface(screen, stone, s[i].x, s[i].y);
	for (int i = 0; i < np; i++)
		if (p[i].IsAlive == true)
			DrawSurface(screen, paddle, p[i].x, p[i].y);
	for (int i = 0; i < nc; i++)
		if (c[i].IsAlive == true)
			DrawSurface(screen, carr2, c[i].x, c[i].y);
	for (int i = 0; i < nt; i++)
		if (t[i].IsAlive == true)
			DrawSurface(screen, tree, t[i].x, t[i].y);
	for (int i = 0; i < number; i++)
		if (b[i].IsAlive == true)
			DrawSurface(screen, bullet, b[i].x, b[i].y);
	DrawSurface(screen, car1, car.x, car.y);
	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
	DrawRectangle(screen, SCREEN_WIDTH - 260, SCREEN_HEIGHT - 40, 260, 40, niebieski, niebieski);
	sprintf(text, "Aleksander Blok s188865, Duration time = %.1lf s  %.0lf points n - new game", worldTime, points);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
	sprintf(text, "Esc - exit, \030 - accelerate, \031 - slowing down s - save game p - pause game");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
	sprintf(text, "Implemented: a,b,c,d,e,f,g,g,i,k");
	DrawString(screen, SCREEN_WIDTH - 260, SCREEN_HEIGHT - 20, text, charset);
}

void UpdatePositions(Object s[], int ns, Object c[], int nc, Object p[], int np,
	Object t[], int nt, Object b[], int& number, Object& car, double& distance, velocity& v, double& delta)
{
	if (distance > SCREEN_HEIGHT + RESETDISTANCE) //If distance is more than 2500 puting objects again
	{
		PutObstacles(s, ns, STONE);
		PutObstacles(c, nc, CAR);
		PutObstacles(p, np, PUDDLE);
		PutObstacles(t, nt, TREE);
		distance = 0;
	}
	distance += v.y * delta;
	if (car.x + v.x < 465 && car.x + v.x > 175)
		car.x += v.x;
	for (int i = 0; i < nc; i++)
		c[i].y = c[i].iy + distance / 2;
	for (int i = 0; i < np; i++)
		p[i].y = p[i].iy + distance;
	for (int i = 0; i < ns; i++)
		s[i].y = s[i].iy + distance;
	for (int i = 0; i < nt; i++)
		t[i].y = t[i].iy + distance;
	for (int i = 0; i < number; i++)
		b[i].y -= (v.y * delta) / 2;
}

void GameRunning(bool& game, int& t1, int& t2, double& delta, SDL_Surface* screen,
	double& distance, velocity& v, double& worldTime, float& points, bool& kill,
	SDL_Surface* stone, SDL_Surface* paddle, SDL_Surface* carr2,
	SDL_Surface* tree, SDL_Surface* bullet, SDL_Surface* car1,
	Object s[], int ns, Object c[], int nc, Object p[], int np,
	Object t[], int nt, Object b[], int& number, Object& car, int czarny,
	int zielony, int czerwony, int niebieski, double& fpsTimer, double& fps,
	int& frames, char text[], bool& pause, Data& Save, SDL_Texture* scrtex,
	SDL_Renderer* renderer, SDL_Surface* charset, int& quit, bool& menu, 
	double& penalty)
{
	if (penalty > PENALTY && kill == true) //Penalty for killing non enemy car
	{
		kill = false;
		penalty = 0;
	}
	t2 = SDL_GetTicks();
	delta = (t2 - t1) * 0.001;
	t1 = t2;
	worldTime += delta;
	if (kill == false)
		points += v.y * delta;
	else
		penalty += delta;
	UpdatePositions(s, ns, c, nc, p, np, t, nt, b, number, car, distance, v, delta);
	DrawObjects(screen, s, ns, c, nc, p, np, t, nt, b, number, car, stone, paddle, 
		carr2, tree, bullet, car1, czarny, zielony, czerwony, niebieski, text, 
		charset, worldTime, points);
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
			case SDLK_UP:
				if (v.y + PlayerSpeedY <= PlayerMaxSpeed)
					v.y += PlayerSpeedY;
				break;
			case SDLK_DOWN:
				if (v.y - PlayerSpeedY >= PlayerMinSpeed)
					v.y -= PlayerSpeedY;
				break;
			case SDLK_RIGHT: v.x = PlayerSpeedX;
				break;
			case SDLK_LEFT: v.x = -PlayerSpeedX;
				break;
			case SDLK_ESCAPE: quit = 1;
				break;
			case SDLK_SPACE:
				b[number].IsAlive = true;
				b[number].y = car.y;
				b[number].x = car.x;
				b[number].c = BULLET;
				number++;
				break;
			case SDLK_p:
				Save.distance = distance;
				Save.velocity = v.y;
				Save.points = points;
				Save.worldtime = worldTime;
				pause = true;
				break;
			case SDLK_s:
				SaveGame(distance, s, ns, c, nc, p, np, t, nt, b, number, car, worldTime, v, points);
				quit = 1;
				break;
			case SDLK_n:
				menu = true;
				break;
			}
			break;
		case SDL_KEYUP:
			switch (event.key.keysym.sym)
			{
			case SDLK_UP:
				break;
			case SDLK_DOWN:
				break;
			case SDLK_RIGHT: v.x = 0;
				break;
			case SDLK_LEFT: v.x = 0;
				break;
			case SDLK_SPACE:
				break;
			}
			break;
		case SDL_QUIT:quit = 1;
			break;
		};
	};
	frames++;
	if (CheckAllCollisions(car, c, s, p, nc, ns, np) == true)
		menu = true;
	CheckKill(c, b, nc, number, kill);
}

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) 
{
	float points;
	int t1, t2, quit, frames, rc, timep, number;
	double delta, worldTime, fpsTimer, fps, distance, penalty;
	const int nc = 2, ns = 5, np = 3, nt = 12, nb = 100;
	bool game, pause, menu, kill;
	SDL_Surface* screen = nullptr, * charset = nullptr;
	SDL_Surface* car1{}, * stone{}, * mainmenu{}, * carr2{}, * paddle{}, * tree{}, * Pause{}, * bullet{};
	SDL_Texture *scrtex{};
	SDL_Window *window{};
	SDL_Renderer *renderer{};
	velocity v;
	Object car;
	Object c[nc];
	Object s[ns];
	Object p[np];
	Object t[nt];
	Object b[nb];
	Data Save{};
	points = 0;
	rc = 0;
	t1 = 0;
	menu = true;
	pause = false;
	game = false;
	quit = 0;
	number = 0;
	penalty = 0;
	kill = false;
	LoadBMP(car1, stone, mainmenu, carr2, paddle, tree, Pause, bullet, scrtex, window, renderer, screen, charset, rc);
	char text[TxtMaxARR];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	const Uint8* keystates = SDL_GetKeyboardState(NULL);
	while(!quit) 
	{
		if (menu == true)
		{
			MainMenu(screen, renderer, mainmenu, game, menu, frames, fpsTimer, fps, 
				points, worldTime, distance, number, v, car, czarny, scrtex, 
				s, c, p, t, ns, nc, np, nt, charset, b, niebieski, kill);
		}
		else if (pause == true)
		{
			PausedGame(pause, t1, t2, delta, screen, Pause, scrtex, renderer, v, Save, distance, points, worldTime);

		}
		else if(game == true)
		{
			GameRunning(game, t1, t2, delta, screen, distance, v, worldTime, points, kill, stone, paddle, carr2, 
				tree, bullet, car1, s, ns, c, nc, p, np, t, nt, b, number, car, czarny, zielony, czerwony, niebieski, fpsTimer, fps, frames,
				text, pause, Save, scrtex, renderer, charset, quit, menu , penalty);
		}
	};
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
	};
