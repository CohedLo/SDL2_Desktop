#include <iostream>
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <vector>

const int WindowWidth = 800;
const int WindowHeight = 600;
const int CellWidth = 50;  // Ширина ячейки
const int CellHeight = 20; // Высота ячейки
const int NumRows = WindowHeight / CellHeight;
const int NumCols = WindowWidth / CellWidth;
const int ColLabelWidth = 20; // Ширина области для буквенных обозначений столбцов
const int RowLabelHeight = 20; // Высота области для числовых обозначений строк

SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;
TTF_Font* g_font = nullptr;

std::string GetColumnLabel(int col) 
{
    std::string label;
    while (col >= 0)
    {
        label = char('A' + col % 26) + label;
        col = col / 26 - 1;
    }
    return label;
}

std::string GetRowLabel(int row)
{
    return std::to_string(row + 1);
}

struct Cell 
{
    std::string content;
    SDL_Rect rect;
};

std::vector<std::vector<Cell>> table(NumRows, std::vector<Cell>(NumCols));

bool InitializeSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    if (TTF_Init() < 0) 
    {
        SDL_Log("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        return false;
    }

    g_window = SDL_CreateWindow("Table Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WindowWidth, WindowHeight, SDL_WINDOW_SHOWN);
    if (g_window == nullptr) 
    {
        SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED);
    if (g_renderer == nullptr)
    {
        SDL_Log("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    g_font = TTF_OpenFont("D:\\SDL2_Desk\\fonts\\ArialRegular.ttf", 10);
    if (g_font == nullptr)
    {
        SDL_Log("Font could not be loaded! TTF_Error: %s\n", TTF_GetError());
        return false;
    }

    for (int row = 0; row < NumRows; ++row) 
    {
        for (int col = 0; col < NumCols; ++col) 
        {
            table[row][col].content = "";
            table[row][col].rect.x = col * CellWidth + ColLabelWidth;
            table[row][col].rect.y = row * CellHeight + RowLabelHeight;
            table[row][col].rect.w = CellWidth;
            table[row][col].rect.h = CellHeight;
        }
    }
    return true;
}

void DrawTable(SDL_Renderer* renderer)
{
    // Очистка экрана (рендеринг белого фона)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // Рисование таблицы (горизонтальные и вертикальные линии)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int x = ColLabelWidth; x <= WindowWidth; x += CellWidth)
    {
        SDL_RenderDrawLine(renderer, x, RowLabelHeight, x, WindowHeight);
    }
    for (int y = RowLabelHeight; y <= WindowHeight; y += CellHeight)
    {
        SDL_RenderDrawLine(renderer, ColLabelWidth, y, WindowWidth, y);
    }

    // Отрисовка содержимого ячеек
    for (int row = 0; row < NumRows; ++row)
    {
        for (int col = 0; col < NumCols; ++col)
        {
            SDL_Color cellTextColor = { 0, 0, 0, 255 };
            std::string cellContent = table[row][col].content;

            // Используем TTF_RenderText_Blended_Wrapped вместо TTF_RenderText_Solid
            SDL_Surface* cellTextSurface = TTF_RenderText_Blended_Wrapped(g_font, cellContent.c_str(), cellTextColor, table[row][col].rect.w - 10);

            SDL_Texture* cellTextTexture = SDL_CreateTextureFromSurface(renderer, cellTextSurface);
            SDL_Rect cellTextRect = { table[row][col].rect.x + 5, table[row][col].rect.y + 5, table[row][col].rect.w - 10, table[row][col].rect.h - 10 };
            SDL_RenderCopy(renderer, cellTextTexture, NULL, &cellTextRect);
            SDL_FreeSurface(cellTextSurface);
            SDL_DestroyTexture(cellTextTexture);
        }
    }
}

void DrawLabels(SDL_Renderer* renderer) 
{
    SDL_Color colTextColor = { 0, 0, 0, 255 };

    // Рисование буквенных обозначений столбцов с отступом вверх и вправо
    for (int col = 0; col < NumCols; ++col) 
    {
        std::string colLabel = GetColumnLabel(col);

        SDL_Surface* colTextSurface = TTF_RenderText_Solid(g_font, colLabel.c_str(), colTextColor);
        SDL_Texture* colTextTexture = SDL_CreateTextureFromSurface(renderer, colTextSurface);
        SDL_Rect colTextRect = { col * CellWidth + ColLabelWidth + (CellWidth - colTextSurface->w) / 2, 0, colTextSurface->w, colTextSurface->h };
        SDL_RenderCopy(renderer, colTextTexture, NULL, &colTextRect);
        SDL_FreeSurface(colTextSurface);
        SDL_DestroyTexture(colTextTexture);
    }

    // Рисование числовых обозначений строк с отступом вверх и влево
    for (int row = 0; row < NumRows; ++row) 
    {
        std::string rowLabel = GetRowLabel(row);

        SDL_Surface* rowTextSurface = TTF_RenderText_Solid(g_font, rowLabel.c_str(), colTextColor);
        SDL_Texture* rowTextTexture = SDL_CreateTextureFromSurface(renderer, rowTextSurface);
        SDL_Rect rowTextRect = { 0, row * CellHeight + RowLabelHeight + (CellHeight - rowTextSurface->h) / 2, ColLabelWidth, rowTextSurface->h };
        SDL_RenderCopy(renderer, rowTextTexture, NULL, &rowTextRect);
        SDL_FreeSurface(rowTextSurface);
        SDL_DestroyTexture(rowTextTexture);
    }
}

void EditCellContents(int mouseX, int mouseY)
{
    for (int row = 0; row < NumRows; ++row)
    {
        for (int col = 0; col < NumCols; ++col)
        {
            if (mouseX >= table[row][col].rect.x && mouseX <= (table[row][col].rect.x + table[row][col].rect.w) &&
                mouseY >= table[row][col].rect.y && mouseY <= (table[row][col].rect.y + table[row][col].rect.h))
            {
                bool editing = true;
                SDL_StartTextInput();

                while (editing)
                {
                    SDL_Event e;
                    while (SDL_PollEvent(&e) != 0)
                    {
                        if (e.type == SDL_QUIT)
                        {
                            editing = false;
                            break;
                        }
                        if (e.type == SDL_KEYDOWN)
                        {
                            if (e.key.keysym.sym == SDLK_RETURN)
                            {
                                editing = false;
                                break;
                            }
                            else if (e.key.keysym.sym == SDLK_BACKSPACE)
                            {
                                // Удаляем последний символ из содержимого ячейки
                                if (!table[row][col].content.empty())
                                {
                                    table[row][col].content.pop_back();
                                }
                            }
                        }
                        if (e.type == SDL_TEXTINPUT)
                        {
                            // Просто добавляем введенный символ
                            table[row][col].content += e.text.text;
                        }
                    }

                    // Очистка экрана
                    SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 255);
                    SDL_RenderClear(g_renderer);

                    DrawTable(g_renderer);
                    DrawLabels(g_renderer);

                    // Отрисовка содержимого редактируемой ячейки
                    SDL_Color cellTextColor = { 0, 0, 0, 255 };
                    std::string cellContent = table[row][col].content;

                    SDL_Surface* cellTextSurface = TTF_RenderText_Blended(g_font, cellContent.c_str(), cellTextColor);
                    SDL_Texture* cellTextTexture = SDL_CreateTextureFromSurface(g_renderer, cellTextSurface);
                    SDL_Rect cellTextRect = { table[row][col].rect.x + 5, table[row][col].rect.y + 5, table[row][col].rect.w - 10, table[row][col].rect.h - 10 };
                    SDL_RenderCopy(g_renderer, cellTextTexture, NULL, &cellTextRect);
                    SDL_FreeSurface(cellTextSurface);
                    SDL_DestroyTexture(cellTextTexture);

                    // Обновление экрана
                    SDL_RenderPresent(g_renderer);
                }
                SDL_StopTextInput();
            }
        }
    }
}





int main(int argc, char* args[]) 
{
    if (!InitializeSDL()) 
    {
        return 1;
    }

    bool quit = false;
    SDL_Event e;

    while (!quit) 
    {
        while (SDL_PollEvent(&e) != 0) 
        {
            if (e.type == SDL_QUIT) 
            {
                quit = true;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) 
            {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                EditCellContents(mouseX, mouseY);
            }
        }

        DrawTable(g_renderer);  // Рисуем таблицу
        DrawLabels(g_renderer); // Рисуем буквенные обозначения столбцов и числовые обозначения строк

        // Отрисовка содержимого ячеек
        for (int row = 0; row < NumRows; ++row) 
        {
            for (int col = 0; col < NumCols; ++col) 
            {
                SDL_Color cellTextColor = { 0, 0, 0, 255 };
                std::string cellContent = table[row][col].content;

                SDL_Surface* cellTextSurface = TTF_RenderText_Solid(g_font, cellContent.c_str(), cellTextColor);
                SDL_Texture* cellTextTexture = SDL_CreateTextureFromSurface(g_renderer, cellTextSurface);
                SDL_Rect cellTextRect = { table[row][col].rect.x + 5, table[row][col].rect.y + 5, table[row][col].rect.w - 10, table[row][col].rect.h - 10 };
                SDL_RenderCopy(g_renderer, cellTextTexture, NULL, &cellTextRect);
                SDL_FreeSurface(cellTextSurface);
                SDL_DestroyTexture(cellTextTexture);
            }
        }
        // Обновление экрана
        SDL_RenderPresent(g_renderer);
    }
    // Очистка ресурсов и завершение работы
    TTF_CloseFont(g_font);
    SDL_DestroyRenderer(g_renderer);
    SDL_DestroyWindow(g_window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}





  