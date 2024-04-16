#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>

#define f32 float
#define ct CellType

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;

typedef enum { CELLTYPE_EMPTY, CELLTYPE_GRASS, CELLTYPE_CONVEYOR } CellType;
typedef enum { ITEM_EMPTY, ITEM_WOOD, ITEM_GOLD } Item;
typedef enum { LEFT, RIGHT, UP, DOWN } Direction;
typedef struct {
  CellType type;
  Item item;
  bool item_updated;
  bool transmutator;
  Direction dir;
} Cell;

#define WORLD_SIZE 50
Cell world[WORLD_SIZE][WORLD_SIZE];

void world_init() {
  for (int y = 0; y < WORLD_SIZE; y++)
    for (int x = 0; x < WORLD_SIZE; x++)
      world[x][y] = (Cell){CELLTYPE_GRASS};

  world[3][3] = (Cell){CELLTYPE_CONVEYOR, ITEM_WOOD, .dir = RIGHT};
  world[4][3] = (Cell){CELLTYPE_CONVEYOR, ITEM_WOOD, .dir = RIGHT};
  world[5][3] = (Cell){CELLTYPE_CONVEYOR, ITEM_WOOD, .dir = DOWN};
  world[5][4] = (Cell){CELLTYPE_CONVEYOR, ITEM_EMPTY, .dir = DOWN};
  world[5][5] = (Cell){CELLTYPE_CONVEYOR, ITEM_EMPTY, .dir = LEFT};
  world[4][5] = (Cell){CELLTYPE_CONVEYOR, ITEM_EMPTY, .dir = LEFT};
  world[3][5] = (Cell){CELLTYPE_CONVEYOR, ITEM_EMPTY, .dir = UP};
  world[3][4] = (Cell){CELLTYPE_CONVEYOR, ITEM_EMPTY, .dir = UP};
}

const int BLOCK_SIZE = 10;
void world_draw() {
  for (int y = 0; y < WORLD_SIZE; y++)
    for (int x = 0; x < WORLD_SIZE; x++) {
      switch (world[x][y].type) {
      case CELLTYPE_EMPTY:
        break;
      case CELLTYPE_GRASS:
        DrawRectangle(x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE,
                      GREEN);
        break;
      case CELLTYPE_CONVEYOR:
        if (world[x][y].transmutator)
          DrawRectangle(x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE,
                        (Color){124, 124, 170, 255});
        else
          DrawRectangle(x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE,
                        (Color){124, 124, 124, 255});
        switch (world[x][y].dir) {
        case LEFT:
          DrawText("<", x * BLOCK_SIZE + 2, y * BLOCK_SIZE - 1, 12, SKYBLUE);
          break;
        case RIGHT:
          DrawText(">", x * BLOCK_SIZE + 2, y * BLOCK_SIZE - 1, 12, SKYBLUE);
          break;
        case UP:
          DrawText("^", x * BLOCK_SIZE + 2, y * BLOCK_SIZE - 1, 12, SKYBLUE);
          break;
        case DOWN:
          DrawText("v", x * BLOCK_SIZE + 2, y * BLOCK_SIZE - 1, 12, SKYBLUE);
          break;
        }
        break;
      }
      switch (world[x][y].item) {
      case ITEM_EMPTY:
        break;
      case ITEM_WOOD:
        DrawRectangle(x * BLOCK_SIZE + 2, y * BLOCK_SIZE + 2, BLOCK_SIZE - 4,
                      BLOCK_SIZE - 4, BROWN);
        break;
      case ITEM_GOLD:
        DrawRectangle(x * BLOCK_SIZE + 2, y * BLOCK_SIZE + 2, BLOCK_SIZE - 4,
                      BLOCK_SIZE - 4, YELLOW);
        break;
      }
    }
}

void cell_update(const int x, const int y) {
  switch (world[x][y].type) {
  case CELLTYPE_EMPTY:
    break;
  case CELLTYPE_GRASS:
    break;
  case CELLTYPE_CONVEYOR:
    if (world[x][y].item && !world[x][y].item_updated) {
      Vector2 next_pos = {0, 0};
      switch (world[x][y].dir) {
      case LEFT:
        next_pos = (Vector2){-1, 0};
        break;
      case RIGHT:
        next_pos = (Vector2){1, 0};
        break;
      case UP:
        next_pos = (Vector2){0, -1};
        break;
      case DOWN:
        next_pos = (Vector2){0, 1};
        break;
      }
      next_pos =
          Vector2Clamp(Vector2Add((Vector2){(f32)x, (f32)y}, next_pos),
                       (Vector2){0, 0}, (Vector2){WORLD_SIZE, WORLD_SIZE});
      if (!world[(int)next_pos.x][(int)next_pos.y].item) {
        if (world[x][y].transmutator)
          if (world[x][y].item == ITEM_WOOD)
            world[x][y].item = ITEM_GOLD;
          else if (world[x][y].item == ITEM_GOLD)
            world[x][y].item = ITEM_WOOD;
        world[(int)next_pos.x][(int)next_pos.y].item = world[x][y].item;
        world[x][y].item = ITEM_EMPTY;
        world[(int)next_pos.x][(int)next_pos.y].item_updated = true;
      } else {
        cell_update(next_pos.x, next_pos.y);
        if (!world[(int)next_pos.x][(int)next_pos.y].item) {
          if (world[x][y].transmutator)
            if (world[x][y].item == ITEM_WOOD)
              world[x][y].item = ITEM_GOLD;
            else if (world[x][y].item == ITEM_GOLD)
              world[x][y].item = ITEM_WOOD;
          world[(int)next_pos.x][(int)next_pos.y].item = world[x][y].item;
          world[x][y].item = ITEM_EMPTY;
          world[(int)next_pos.x][(int)next_pos.y].item_updated = true;
        }
      }
    }
  }
}

void world_update() {
  const double tick_interval = 0.5;
  static double last_tick_timestamp = 0;

  if (GetTime() - last_tick_timestamp >= tick_interval) {
    last_tick_timestamp = GetTime();
    for (int y = 0; y < WORLD_SIZE; y++)
      for (int x = 0; x < WORLD_SIZE; x++)
        world[x][y].item_updated = false;

    for (int y = 0; y < WORLD_SIZE; y++)
      for (int x = 0; x < WORLD_SIZE; x++) {
        cell_update(x, y);
      }
  }
}

struct {
  Vector2 pos;
  int SIZE;
  int SPEED;
  Camera2D camera;
} player;

void player_init() {
  player.SIZE = 8;
  player.SPEED = 30;
  player.pos =
      (Vector2){WORLD_SIZE * BLOCK_SIZE / 2, WORLD_SIZE * BLOCK_SIZE / 2};
  player.camera = (Camera2D){0};
  player.camera.offset = (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
  player.camera.zoom = 8.0f;
}

void player_draw() {
  DrawRectangle(player.pos.x, player.pos.y, player.SIZE, player.SIZE, RED);
}

void player_editor() {
  Vector2 mpos = GetScreenToWorld2D(GetMousePosition(), player.camera);

  int mpos_x = (int)(mpos.x / BLOCK_SIZE);
  int mpos_y = (int)(mpos.y / BLOCK_SIZE);

  if (mpos_x < 0 || mpos_y < 0)
    return;
  if (mpos_x >= WORLD_SIZE || mpos_y >= WORLD_SIZE)
    return;

  bool shift_pressed = IsKeyDown(KEY_LEFT_SHIFT);

  if (IsKeyDown(KEY_UP)) {
    world[mpos_x][mpos_y] = (Cell){CELLTYPE_CONVEYOR, ITEM_EMPTY, .dir = UP,
                                   .transmutator = shift_pressed};
  }
  if (IsKeyPressed(KEY_DOWN)) {
    world[mpos_x][mpos_y] = (Cell){CELLTYPE_CONVEYOR, ITEM_EMPTY, .dir = DOWN,
                                   .transmutator = shift_pressed};
  }
  if (IsKeyPressed(KEY_LEFT)) {
    world[mpos_x][mpos_y] = (Cell){CELLTYPE_CONVEYOR, ITEM_EMPTY, .dir = LEFT,
                                   .transmutator = shift_pressed};
  }
  if (IsKeyPressed(KEY_RIGHT)) {
    world[mpos_x][mpos_y] = (Cell){CELLTYPE_CONVEYOR, ITEM_EMPTY, .dir = RIGHT,
                                   .transmutator = shift_pressed};
  }
  if (IsKeyPressed(KEY_ONE)) {
    world[mpos_x][mpos_y].item = ITEM_WOOD;
  }
  if (IsKeyPressed(KEY_TWO)) {
    world[mpos_x][mpos_y] = (Cell){CELLTYPE_GRASS};
  }
}

void player_move() {
  float delta = GetFrameTime();

  Vector2 dir = Vector2Normalize(
      (Vector2){.x = (f32)IsKeyDown(KEY_D) - (f32)IsKeyDown(KEY_A),
                .y = (f32)IsKeyDown(KEY_S) - (f32)IsKeyDown(KEY_W)});

  Vector2 velocity = Vector2Scale(dir, delta * player.SPEED);
  player.pos = Vector2Add(player.pos, velocity);

  // Camera zoom controls
  player.camera.zoom += ((f32)GetMouseWheelMove() * 0.05f);

  if (player.camera.zoom > 3.0f)
    player.camera.zoom = 3.0f;
  else if (player.camera.zoom < 0.1f)
    player.camera.zoom = 0.1f;
}

int main(void) {
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "factory");
  // SetTargetFPS(60);

  world_init();
  player_init();

  while (!WindowShouldClose()) {
    player_move();
    world_update();
    player_editor();
    BeginDrawing();
    ClearBackground(BLACK);
    player.camera.target = (Vector2){player.pos.x + player.SIZE / 2,
                                     player.pos.y + player.SIZE / 2};
    BeginMode2D(player.camera);
    world_draw();
    player_draw();
    EndMode2D();
    DrawFPS(0, 0);
    EndDrawing();
  }
  CloseWindow();
}