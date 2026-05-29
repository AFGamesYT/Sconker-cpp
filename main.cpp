#include "raylib.h"
#include <bits/stdc++.h>

enum Level {
    MAIN_MENU,
    SAVE_POPUP,
    GAME,
    PAUSED_GAME,
    SETTINGS,
    EXIT_CONFIRMATION,
    TEST // for beta, remove in release build
};

Level lastFrameLevel = MAIN_MENU;
Level currentLevel = MAIN_MENU;

enum TimeOfDay {
    SUNRISE,
    DAYTIME,
    SUNSET,
    NIGHTTIME
};

enum Season {
    SUMMER,
    AUTUMN,
    WINTER,
    SPRING
};

enum TextureID {
    CONKER,
    SHADOW,
    GROUND,
    SUN,
    TREE,
    SKY_DAY,
    SKY_NIGHT,
    CONKER_BREAK_SPRITES,
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
    textures[SHADOW]           = LoadTexture("assets/conker/shadow.png");
    
    textures[GROUND]           = LoadTexture("assets/environment/ground.png");
    textures[SUN]              = LoadTexture("assets/environment/sun.png"); 
    textures[TREE]             = LoadTexture("assets/environment/tree.png");
    
    textures[SKY_DAY]          = LoadTexture("assets/environment/sky/background_day.png");
    textures[SKY_NIGHT]        = LoadTexture("assets/environment/sky/background_night.png");

    textures[CONKER_BREAK_SPRITES] = LoadTexture("assets/conker/break_spritesheet.png");
    
    textures[MENU_BG]          = LoadTexture("assets/gui/menu_background.png");
    textures[LOGO]             = LoadTexture("assets/gui/sconker_logo_no_bg.png");
    textures[SETTINGS_BG]      = LoadTexture("assets/gui/settings_background.png");
    
    textures[PLAY_BUTTON]      = LoadTexture("assets/gui/buttons/play_button.png");
    textures[EXIT_BUTTON]      = LoadTexture("assets/gui/buttons/exit_button.png");
    textures[SETTINGS_BUTTON]  = LoadTexture("assets/gui/buttons/settings_button.png");

    textures[CLOUD1]           = LoadTexture("assets/environment/clouds/cloud1.png");
    textures[STUMP]            = LoadTexture("assets/gui/stump.png");
}

// TODO - redraw following textures:
// GROUND, TREE, SUN, SKY_DAY, SKY_NIGHT

// TODO - draw following textures:
// FARMER_CHARACTER, DAY_SKY_OVERLAY, NIGHT_SKY_OVERLAY, DNA


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
        bool isBasic = true;

        // basic anim
        float current;
        float start;
        float end;
        
        // spritesheet anim
        int frames;
        int frameWidth;
        int frameHeight;
        bool loop;

        // all anims
        double startTime;
        float length;
        double timePassed;

        AnimObject(float start, float end, float length, float offset)
        {   
            this->start = start;
            this->end = end;
            this->length = length;

            startTime = GetTime() - offset;
            timePassed = 0;
        }

        AnimObject(int frames, int frameWidth, int frameHeight, float length, bool loop, float offset) 
        {
            isBasic = false;

            this->frames = frames;
            this->frameWidth = frameWidth;
            this->frameHeight = frameHeight;
            this->length = length;
            this->loop = loop;

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

            if (!anim.isBasic) return 0.0f;

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
            
            if (!anim.isBasic) return 0.0f;

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

            if (!anim.isBasic) return 0.0f;

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

            if (!anim.isBasic) return 0.0f;

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
        
        Rectangle spriteAnim(int id) {
            AnimObject &anim = playingAnims.at(id);

            if (!anim.isBasic) {
                float t = (GetTime() - anim.startTime) / anim.length;
                
                int i = t*anim.frames;
                if (t >= 1) {
                    if (anim.loop) {
                        t = std::fmod(t, 1);
                    } else {
                        i = anim.frames - 1;
                    }
                }

                anim.timePassed = GetTime() - anim.startTime;

                return Rectangle{(float)i*anim.frameWidth, 0, (float)anim.frameWidth, (float)anim.frameHeight};
            }
        }

        void createAnim(int id, float startV, float endV, float length, float offset = 0.0f) { // create a basic anim | quadratic, linear
            if (playingAnims.count(id) != 1) {
                playingAnims.insert({id, AnimObject(startV, endV, length, offset)});
            } 
        }

        void createSpriteAnim(int id, int frameAmount, int frameW, int frameH, float length, bool loop = false, float offset = 0.0f) { // create a spritesheet anim | spriteAnim()
            if (playingAnims.count(id) != 1) {
                playingAnims.insert({id, AnimObject(frameAmount, frameW, frameH, length, loop, offset)});
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

void handleMenu() {
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
    if (hoveringTexture(textures[PLAY_BUTTON], Vector2{scrDimensions.x*0.073f, scrDimensions.y*0.01f}, scrDimensions.x/1200.0f) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        currentLevel = GAME;
    }

    if (hoveringTexture(textures[SETTINGS_BUTTON], Vector2{scrDimensions.x*0.045f, scrDimensions.y*0.36f}, scrDimensions.x/1400.0f) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        currentLevel = SETTINGS;
    }

    if (hoveringTexture(textures[EXIT_BUTTON], Vector2{scrDimensions.x*0.083f, scrDimensions.y*0.67f}, scrDimensions.x/1150.0f) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        currentLevel = EXIT_CONFIRMATION;
    }
}

void handleSettings() { // placeholder for now
    Vector2 scrDimensions = {GetScreenWidth(), GetScreenHeight()};

    DrawTextureRec(textures[SETTINGS_BG], Rectangle{0, 0, scrDimensions.x, scrDimensions.y}, Vector2{0, 0}, WHITE);
}

void handleTest() { // remove in release
    Vector2 scrDimensions = {GetScreenWidth(), GetScreenHeight()};

    DrawTextureRec(textures[SKY_DAY], Rectangle{0, 0, scrDimensions.x, scrDimensions.y}, Vector2{0, 0}, WHITE);

    animHandler.createSpriteAnim(6, 12, 500, 500, 1.5, true);

    Rectangle rec = animHandler.spriteAnim(6);

    DrawTextureRec(textures[CONKER_BREAK_SPRITES], rec, Vector2{0, 0}, WHITE);
}


void handleGame() {
    /*
    game updates happen 20 times every seconds, so ups = 20 (updates per second)
    visual, input updates happen every frame
    */

    static double accumulator;

    static int totalUpdates; // get from save file

    const float tickTime = 0.05f; // 1/20 = 0.05
    float frameTime = GetFrameTime();

    static Season currentSeason = SUMMER; // based on the time gotten from the save file
    static TimeOfDay currentTimeOfDay = DAYTIME;

    Vector2 scrDimensions = {GetScreenWidth(), GetScreenHeight()};


    accumulator += frameTime;

    if (accumulator >= tickTime) {
        // ------------------------------
        // |       GAME UPDATES         |
        // ------------------------------
        
        // 10 minutes  = 1 day ( 1 - sunrise / 4 - day / 1 - sunset / 4 - night )
        // 0-1 = sunrise | 1-5 = day | 5-6 = sunset | 6-10 = night

        // 1 season    = 7 days
        // 1 year      = 28 days

        int yearTime = totalUpdates % (28*10*60*20); // 28 days * 10 minutes * 60 seconds * 20 updates | num range = [ 0, 336000 )
        int dayTime =  totalUpdates % (10*60*20);    // 1 day   * 10 minutes * 60 seconds * 20 updates | num range = [ 0, 12000  )

        if (dayTime <= 60*20) {   

            if (currentTimeOfDay != SUNRISE) {
                currentTimeOfDay = SUNRISE;
            }

        } else if (dayTime <= 5*60*20) { 

            if (currentTimeOfDay != DAYTIME) {
                currentTimeOfDay = DAYTIME;
            }

        } else if (dayTime <= 6*60*20) {

            if (currentTimeOfDay != SUNSET) {
                currentTimeOfDay = SUNSET;
            }

        } else {

            if (currentTimeOfDay != NIGHTTIME) {
                currentTimeOfDay = NIGHTTIME;
            }

        }   


        if (yearTime <= 7*10*60*20) { // summer [0, 84000]

            if (currentSeason != SUMMER) {
                currentSeason = SUMMER;
            }

        } else if (yearTime <= 14*10*60*20) { // autumn 

            if (currentSeason != AUTUMN) {
                currentSeason = AUTUMN;
            }

        } else if (yearTime <= 21*10*60*20) { // winter

            if (currentSeason != WINTER) {
                currentSeason = WINTER;
            }

        } else { // spring

            if (currentSeason != SPRING) {
                currentSeason = SPRING;
            }

        }
        
        TraceLog(LOG_DEBUG, std::to_string(dayTime).c_str());

        totalUpdates += 1; // 2x, 3x speed mode/upgrade ?
        accumulator -= tickTime;
    }

    // ------------------------------
    // |       INPUT UPDATE        |
    // ------------------------------



    // ------------------------------
    // |       VISUAL UPDATE        |
    // ------------------------------
    
    switch (currentTimeOfDay) {
        case DAYTIME:
            DrawTextureRec(textures[SKY_DAY], Rectangle{0, 0, scrDimensions.x, scrDimensions.y}, Vector2{0, 0}, WHITE);
            break;
        
        case NIGHTTIME:
            DrawTextureRec(textures[SKY_NIGHT], Rectangle{0, 0, scrDimensions.x, scrDimensions.y}, Vector2{0, 0}, WHITE);
            break;
        
        case SUNSET:
            DrawTextureRec(textures[SKY_NIGHT], Rectangle{0, 0, scrDimensions.x, scrDimensions.y}, Vector2{0, 0}, WHITE);
            DrawTexture(textures[CONKER], 0, 0, WHITE); // placeholder to identify it changed
            break;
        
        case SUNRISE:
            DrawTextureRec(textures[SKY_NIGHT], Rectangle{0, 0, scrDimensions.x, scrDimensions.y}, Vector2{0, 0}, WHITE);
            DrawTexture(textures[CLOUD1], 0, 0, WHITE); // placeholder to identify it changed
            break;

        default:
            break;
        }

    
}

int main()
{   
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    bool running = true;

    SetTargetFPS(0); // uncap fps
    SetTraceLogLevel(LOG_ALL);

    SetConfigFlags(FLAG_VSYNC_HINT); // enables vsync
    InitWindow(screenWidth, screenHeight, "Sconker");

    load();

    SetExitKey(KEY_NULL);

    while (!WindowShouldClose() && running) {
        BeginDrawing();

        switch (currentLevel) {
            case MAIN_MENU: 
                handleMenu();
                break;

            case GAME:
                handleGame();
                break;

            case SETTINGS: 
                handleSettings();
                break;

            case EXIT_CONFIRMATION:
                running = false; // TODO - make this an actual exit confirmation

            case TEST:
                handleTest();
                break;

            default:
                currentLevel = MAIN_MENU;
                handleMenu();
        }
    
        EndDrawing();
        lastFrameLevel = currentLevel;
    }

    unload();
    CloseWindow();

    return 0;
}
