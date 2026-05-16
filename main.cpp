#include "raylib.h"
#include <bits/stdc++.h>

enum Level {
    MAIN_MENU = 0,
    LOAD_CONTINUE_GAME = 1,
    GAME = 2,
    PAUSED_GAME = 3,
    DNA_UPGRADES = 4,
    CLEANER_DEFENSE = 5,
    SETTINGS = 6,
    EXIT_CONFIRMATION = 7
};

Level currentLevel = MAIN_MENU;

enum TextureID {
    CONKER,
    CONKER_LEFT,
    CONKER_RIGHT,
    SHADOW,
    GROUND,
    SUN,
    TREE,
    SKY_DAY,
    SKY_NIGHT,
    MENU_BG,
    LOGO,
    SETTINGS_BG,
    STUMP,
    PLAY_BUTTON,
    SETTINGS_BUTTON,
    EXIT_BUTTON,
    CLOUD1,
    TEXTURE_COUNT
};

Texture2D textures[TEXTURE_COUNT];

void load()
{
    textures[CONKER]           = LoadTexture("assets/conker/conker.png");
    textures[CONKER_LEFT]      = LoadTexture("assets/conker/conker_left.png");
    textures[CONKER_RIGHT]     = LoadTexture("assets/conker/conker_right.png");
    textures[SHADOW]           = LoadTexture("assets/conker/shadow.png");
    
    textures[GROUND]           = LoadTexture("assets/environment/ground.png");
    textures[SUN]              = LoadTexture("assets/environment/sun.png"); // TODO - redraw this texture
    textures[TREE]             = LoadTexture("assets/environment/tree.png");
    
    textures[SKY_DAY]          = LoadTexture("assets/environment/sky/background_day.png");
    textures[SKY_NIGHT]        = LoadTexture("assets/environment/sky/background_night.png");
    
    textures[MENU_BG]          = LoadTexture("assets/gui/menu_background.png");
    textures[LOGO]             = LoadTexture("assets/gui/sconker_logo_no_bg.png");
    textures[SETTINGS_BG]      = LoadTexture("assets/gui/settings_background.png");
    
    textures[PLAY_BUTTON]      = LoadTexture("assets/gui/buttons/play_button.png");
    textures[EXIT_BUTTON]      = LoadTexture("assets/gui/buttons/exit_button.png");
    textures[SETTINGS_BUTTON]  = LoadTexture("assets/gui/buttons/settings_button.png");

    textures[CLOUD1]           = LoadTexture("assets/environment/clouds/cloud1.png");
    textures[STUMP]            = LoadTexture("assets/gui/stump.png"); // TODO - redraw this texture maybe
}

void unload()
{
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        UnloadTexture(textures[i]);
    }
}


// helper functions

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

bool hoveringTexture(Texture2D tex, Vector2 pos, float scale) {
    Vector2 mouse = GetMousePosition();

    return !(mouse.x < pos.x || mouse.y < pos.y ||
             mouse.x > pos.x + tex.width * scale ||
             mouse.y > pos.y + tex.height * scale);
}

float clamp(float num, float limitA, float limitB) {
    if (num >= limitA && num <= limitB) {
        return num;
    } else if (num < limitA) {
        return limitA;
    } else if (num > limitB) {
        return limitB;
    } else {
        return num;
    }
}

// animation classes
class AnimObject {
    public:
        float current;
        float start;
        float end;
        
        double startTime;
        float length;
        double timePassed;

        AnimObject(float startV, float endV, float len, float offset) {

            start = startV;
            end = endV;
            length = len;

            startTime = GetTime() - offset;
            timePassed = 0;
        }
};

class AnimHandler {
    private:
        std::map<int, AnimObject> playingAnims;

    public:
        float quadraticInOut(int id) {
            AnimObject &anim = playingAnims.at(id);

            float t = (GetTime() - anim.startTime) / anim.length;

            float x;
            if (t >= 1) {
                x = 1;
            } else {
                x = (t < 0.5) ? t*t*2 : -1+(4-2*t)*t;
            }

            anim.timePassed = GetTime() - anim.startTime;

            anim.current = anim.start + (anim.end - anim.start) * x;

            return anim.start + (anim.end - anim.start) * x;
        }

        float quadraticIn(int id) {
            AnimObject &anim = playingAnims.at(id);
            
            float t = (GetTime() - anim.startTime) / anim.length;

            
            float x;
            if (t >= 1) {
                x = 1;
            } else {
                x = t*t;
            }

            anim.timePassed = GetTime() - anim.startTime;

            anim.current = anim.start + (anim.end - anim.start) * x;

            return anim.start + (anim.end - anim.start) * x;
        }

        float quadraticOut(int id) {
            AnimObject &anim = playingAnims.at(id);

            float t = (GetTime() - anim.startTime) / anim.length;

            float x;
            if (t >= 1) {
                x = 1;
            } else {
                x = t * (2 - t);
            }

            anim.timePassed = GetTime() - anim.startTime;

            anim.current = anim.start + (anim.end - anim.start) * x;

            return anim.start + (anim.end - anim.start) * x;
        }

        float linear(int id) {
            AnimObject &anim = playingAnims.at(id);

            float t = (GetTime() - anim.startTime) / anim.length;

            float x;
            if (t >= 1) {
                x = 1;
            } else {
                x = t;
            }

            anim.timePassed = GetTime() - anim.startTime;

            anim.current = anim.start + (anim.end - anim.start) * x;

            return anim.start + (anim.end - anim.start) * x;
        }
    
        void createAnim(int id, float startV, float endV, float length, float offset = 0.0f) {
            if (playingAnims.count(id) != 1) {
                playingAnims.insert({id, AnimObject(startV, endV, length, offset)});
            } 
        }

        void overrideAnim(int id, float startV, float endV, float length, float offset = 0.0f) {
            playingAnims.at(id) = AnimObject(startV, endV, length, offset);
        }

        AnimObject getAnim(int id) {
            return playingAnims.at(id);
        }

        bool animFinished(int id) {
            return playingAnims.at(id).timePassed >= playingAnims.at(id).length;
        }

        void stopAnim(int id) {
            if (playingAnims.count(id) > 0) playingAnims.erase(id);
        }

        void stopAnims(std::vector<int> ids) {
            for (int id : ids) {
                stopAnim(id);
            }
        }
};

AnimHandler animHandler;

void drawButton(int id, Texture2D texture, Vector2 buttonPos, float startScale, float endScale, float length)
{
    bool isHovering = hoveringTexture(texture, buttonPos, startScale);

    static std::unordered_map<int, bool> wasHovering;

    animHandler.createAnim(id, startScale, startScale, length);

    float animLen = clamp(length - animHandler.getAnim(id).timePassed, 0, length);
    animLen = length - animLen;

    if (isHovering && !wasHovering[id]) {
        animHandler.overrideAnim(
            id,
            animHandler.getAnim(id).current,
            endScale,
            animLen
        );
    }

    if (!isHovering && wasHovering[id]) {
        animHandler.overrideAnim(
            id,
            animHandler.getAnim(id).current,
            startScale,
            animLen
        );
    }

    float scale = animHandler.quadraticInOut(id);

    wasHovering[id] = isHovering;

    buttonPos.x -= (scale * texture.width - startScale * texture.width) / 2;
    buttonPos.y -= (scale * texture.height - startScale * texture.height) / 2;

    DrawTextureEx(
        texture,
        buttonPos,
        0.0f,
        scale,
        WHITE
    );
}
// draw levels

void handleMenu()
{
    Vector2 scrDimensions = {GetScreenWidth(), GetScreenHeight()};

    // bg
    DrawTextureRec(textures[SKY_DAY], Rectangle{0, 0, scrDimensions.x, scrDimensions.y}, Vector2{0, 0}, WHITE);

    // clouds
    animHandler.createAnim(4, scrDimensions.x*0.25f, scrDimensions.x*0.4f, 27);
    
    if (animHandler.animFinished(4)) {
        animHandler.overrideAnim(4, animHandler.getAnim(4).end, animHandler.getAnim(4).start, 27);
    }

    DrawTextureEx(textures[CLOUD1], Vector2{animHandler.quadraticInOut(4), scrDimensions.y*0.2f}, 0, scrDimensions.x/1400.0f, WHITE);


    animHandler.createAnim(5, scrDimensions.x*0.45f, scrDimensions.x*0.35f, 21, 3);
    
    if (animHandler.animFinished(5)) {
        animHandler.overrideAnim(5, animHandler.getAnim(5).end, animHandler.getAnim(5).start, 21);
    }

    DrawTextureEx(textures[CLOUD1], Vector2{animHandler.quadraticInOut(5), scrDimensions.y*0.6f}, 0, scrDimensions.x/1400.0f, WHITE); // TODO - draw new cloud texture, make it unique

    // static things
    DrawTextureEx(textures[LOGO], Vector2{scrDimensions.x*0.4f, scrDimensions.y*0.15f}, 0.0f, scrDimensions.x/10000*4, WHITE); // logo
    DrawTextureEx(textures[CONKER], Vector2{scrDimensions.x*0.6f, scrDimensions.y*0.69f}, 0.0f, scrDimensions.x/10000*7.1, WHITE); // conker
    DrawTextureEx(textures[STUMP], Vector2{scrDimensions.x*0.1f, 0}, 0.0f, scrDimensions.y/1958.0f, WHITE); // stump

    
    char *verText = "beta 0.1";
    int size = scrDimensions.y*0.03;
    DrawText(verText, scrDimensions.x-MeasureText(verText, size)-10, scrDimensions.y-size, size, WHITE);

    // buttons

    drawButton(1, textures[PLAY_BUTTON], Vector2{scrDimensions.x*0.073f, scrDimensions.y*0.01f}, scrDimensions.x/1300.0f, scrDimensions.x/1200.0f, 0.2f); // play
    drawButton(2, textures[SETTINGS_BUTTON], Vector2{scrDimensions.x*0.045f, scrDimensions.y*0.36f}, scrDimensions.x/1400.0f, scrDimensions.x/1300.0f, 0.2f); // settings
    drawButton(3, textures[EXIT_BUTTON], Vector2{scrDimensions.x*0.083f, scrDimensions.y*0.67f}, scrDimensions.x/1250.0f, scrDimensions.x/1150.0f, 0.2f); // exit

    // handle button presses
    if (hoveringTexture(textures[SETTINGS_BUTTON], Vector2{scrDimensions.x*0.045f, scrDimensions.y*0.36f}, scrDimensions.x/1400.0f) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        currentLevel = SETTINGS;
    }

    if (hoveringTexture(textures[EXIT_BUTTON], Vector2{scrDimensions.x*0.083f, scrDimensions.y*0.67f}, scrDimensions.x/1150.0f) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        currentLevel = EXIT_CONFIRMATION;
    }
}

void handleSettings() {
    Vector2 scrDimensions = {GetScreenWidth(), GetScreenHeight()};

    DrawTextureRec(textures[SETTINGS_BG], Rectangle{0, 0, scrDimensions.x, scrDimensions.y}, Vector2{0, 0}, WHITE); // bg
}


int main()
{   
    // CONSTS

    const int screenWidth = 1920;
    const int screenHeight = 1080;
    const int fps = 144; 

    SetTargetFPS(fps);

    SetConfigFlags(FLAG_VSYNC_HINT); // enable vsync
    InitWindow(screenWidth, screenHeight, "Sconker");

    load();

    while (!WindowShouldClose()) {
        BeginDrawing();

        switch (currentLevel) {
            case MAIN_MENU: 
                handleMenu();
                break;
            case SETTINGS: 
                handleSettings();
                break;
            case EXIT_CONFIRMATION:
                unload();
                CloseWindow();
            default:
                currentLevel = MAIN_MENU;
                handleMenu();
        }

        EndDrawing();
    }

    unload();
    CloseWindow();

    return 0;
}
