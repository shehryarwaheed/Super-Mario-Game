//  Super Mario
//  Original authors: L24-3023 Muhammad Shehryar Waheed
//                    L24-3007 Husnain Khan
//                    L24-3063 Muazam Mehmood

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <sstream>
#include <fstream>
#include <iostream>
using namespace std;

// Opens a name-entry window and returns the (uppercased) name the player types.
string askname(sf::Font& font, string space)
{
    char name[100] = "";
    int  namelength = 0;
    space = " ";

    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Asking Name of the player");

    sf::Text question;
    question.setFont(font);
    question.setCharacterSize(70);
    question.setFillColor(sf::Color::Yellow);
    question.setPosition(500, 400);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::TextEntered)
            {
                if (event.text.unicode == '\b' && namelength > 0)
                {
                    namelength--;
                    name[namelength] = '\0';
                }
                else if (event.text.unicode == '\r')
                {
                    for (int i = 0; i < namelength; i++)
                        name[i] = toupper(name[i]);
                    return name;
                }
                else if ((event.text.unicode < 128) && namelength < 99)
                {
                    name[namelength] = static_cast<char>(event.text.unicode);
                    namelength++;
                    name[namelength] = '\0';
                }
            }
            question.setString("What is your name? " + space + "\nYour Name:   " + name);
        }

        window.clear();
        window.draw(question);
        window.display();
    }
    return 0;
}

// Inserts newName/newScore into the top-5 leaderboard file if the score qualifies.
void updateLeaderboard(string filename, string newName, int newScore)
{
    const int size = 5;
    string names[size];
    int    scores[size];

    ifstream inputFile(filename);
    if (!inputFile) { cout << "Error opening file!" << endl; return; }
    for (int i = 0; i < size; i++) inputFile >> names[i] >> scores[i];
    inputFile.close();

    int position = -1;
    for (int i = 0; i < size; i++)
    {
        if (newScore < scores[i]) { position = i; break; }
    }
    if (position == -1) { cout << "Score not high enough to update the leaderboard." << endl; return; }

    for (int i = size - 1; i > position; i--)
    {
        names[i]  = names[i - 1];
        scores[i] = scores[i - 1];
    }
    names[position]  = newName;
    scores[position] = newScore;

    ofstream outputFile(filename);
    if (!outputFile) { cout << "Error opening file!" << endl; return; }
    for (int i = 0; i < size; i++)
        outputFile << names[i] << " " << scores[i] << endl;
    outputFile.close();

    cout << "Leaderboard updated successfully!" << endl;
}

// Reads the top-5 leaderboard and stores formatted strings in leader[0..4].
void leaderboardscore(string filename, string* leader)
{
    const int size = 5;
    string names[size];
    int    scores[size];

    ifstream inputFile(filename);
    if (!inputFile) { cout << "Error opening file!" << endl; return; }
    for (int i = 0; i < size; i++) inputFile >> names[i] >> scores[i];
    inputFile.close();

    for (int i = 0; i < size; i++)
        leader[i] = names[i] + "    " + to_string(scores[i]) + "\n";
}

// Scales a sprite so it fills the entire window.
void fitToWindow(sf::Sprite& sprite, const sf::RenderWindow& window)
{
    sprite.setScale(
        window.getSize().x / sprite.getGlobalBounds().width,
        window.getSize().y / sprite.getGlobalBounds().height
    );
}

bool loadTexture(sf::Texture& tex, const string& path)
{
    if (!tex.loadFromFile(path))
    {
        cerr << "Failed to load texture: " << path << endl;
        return false;
    }
    return true;
}

// Opens a music file and applies common settings; returns false on failure.
bool loadMusic(sf::Music& music, const string& path, float volume, bool loop = false)
{
    if (!music.openFromFile(path))
    {
        cerr << "Failed to open audio: " << path << endl;
        return false;
    }
    music.setVolume(volume);
    music.setLoop(loop);
    return true;
}

// Applies gravity and vertical movement, snaps player to the ground floor.
void applyGravityAndGround(sf::Sprite& player, float& velocityY,
                            bool& isjumping, bool affectGravity,
                            float level, float gravity)
{
    if (affectGravity)
        velocityY += gravity;

    player.move(0, velocityY);

    if (player.getPosition().y >= level)
    {
        player.setPosition(player.getPosition().x, level);
        isjumping = false;
    }
}

// Handles left/right keyboard input and updates textures.
void handleHorizontalMovement(sf::Sprite& player, bool& jumpside,
                               float moveSpeed,
                               const sf::Texture& leftTex, const sf::Texture& rightTex,
                               const sf::RenderWindow& window)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
    {
        jumpside = false;
        if (player.getPosition().x >= 0)
        {
            player.move(-moveSpeed, 0);
            player.setTexture(leftTex);
        }
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
    {
        jumpside = true;
        if (player.getPosition().x + player.getGlobalBounds().width <= window.getSize().x)
        {
            player.move(moveSpeed, 0);
            player.setTexture(rightTex);
        }
    }
}

// Starts a jump if the player is on the ground and Space/Up is pressed.
void handleJumpInput(sf::Sprite& player, bool& isjumping, bool& jumpside,
                     float& velocityY, float jumpSpeed, sf::Music& jumpSound,
                     const sf::Texture& leftJumpTex, const sf::Texture& rightJumpTex)
{
    if (!isjumping &&
        (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ||
         sf::Keyboard::isKeyPressed(sf::Keyboard::Up)))
    {
        jumpSound.play();
        isjumping = true;
        velocityY = jumpSpeed;
        player.setTexture(jumpside ? rightJumpTex : leftJumpTex);
    }
}

// Switches between idle textures when not jumping.
void updateIdleTexture(sf::Sprite& player, bool isjumping, bool jumpside,
                       const sf::Texture& leftIdle, const sf::Texture& rightIdle)
{
    if (!isjumping)
        player.setTexture(jumpside ? rightIdle : leftIdle);
}

// Generic AABB collision response for a rectangular platform (top/bottom/left/right).
void resolveBlockCollision(sf::Sprite& player, float& velocityY, bool& isjumping,
                            const sf::Sprite& block, float threshold = 5.f)
{
    if (!player.getGlobalBounds().intersects(block.getGlobalBounds()))
        return;

    auto pb = player.getGlobalBounds();
    auto bb = block.getGlobalBounds();

    // Top
    if (pb.top + pb.height <= bb.top + threshold &&
        pb.top + pb.height >  bb.top)
    {
        player.setPosition(player.getPosition().x, bb.top - pb.height);
        velocityY = 0;
        isjumping = false;
        return;
    }
    // Bottom
    if (pb.top >= bb.top + bb.height - threshold &&
        pb.top <  bb.top + bb.height + 20)
    {
        player.setPosition(player.getPosition().x, bb.top + bb.height);
        return;
    }
    // Left
    if (pb.left + pb.width <= bb.left + 25 &&
        pb.left + pb.width >  bb.left)
    {
        player.setPosition(bb.left - pb.width, player.getPosition().y);
        return;
    }
    // Right
    if (pb.left >= bb.left + bb.width - 25 &&
        pb.left <  bb.left + bb.width)
    {
        player.setPosition(bb.left + bb.width, player.getPosition().y);
    }
}

bool handleEnemyCollision(sf::Sprite& player, sf::Sprite& enemy,
                           bool isjumping, bool& isalive, int& life, int& score,
                           int& enemycount, sf::Clock& deathtimer,
                           sf::Music& smashSound, sf::Music& hitSound,
                           float level)
{
    if (!player.getGlobalBounds().intersects(enemy.getGlobalBounds()))
        return false;

    auto pb = player.getGlobalBounds();
    auto eb = enemy.getGlobalBounds();

    if ((pb.top + pb.height <= eb.top + 5) && isjumping)
    {
        smashSound.play();
        score += 10;
        enemy.setPosition(-100, -100);
        enemycount++;
        return true;
    }
    else if (!isalive)
    {
        hitSound.play();
        life--;
        player.setPosition(100, level - 200);
        deathtimer.restart();
        isalive = true;
    }
    return false;
}

// Resets the isalive flag after the invincibility window expires.
void updateInvincibility(bool& isalive, const sf::Clock& deathtimer)
{
    if (isalive && deathtimer.getElapsedTime().asSeconds() > 1.f)
        isalive = false;
}

// Hides heart sprites according to the remaining lives count.
void updateHearts(int life, bool& isdead,
                  sf::Sprite& h1, sf::Sprite& h2, sf::Sprite& h3)
{
    if (life == 2)
        h3.setPosition(-100, -100);
    else if (life == 1)
    {
        h3.setPosition(-100, -100);
        h2.setPosition(-100, -100);
    }
    else if (life == 0)
    {
        h3.setPosition(-100, -100);
        h2.setPosition(-100, -100);
        h1.setPosition(-100, -100);
        isdead = true;
    }
}

// Builds the "MM:SS" timer string and writes it to a Text object.
void updateTimerDisplay(const sf::Clock& gametime, sf::Text& gametimer,
                        ostringstream& timeStream)
{
    sf::Time elapsed = gametime.getElapsedTime();
    int minutes = static_cast<int>(elapsed.asSeconds()) / 60;
    int seconds = static_cast<int>(elapsed.asSeconds()) % 60;
    timeStream.str("");
    timeStream << "Time: " << minutes << ":" << (seconds < 10 ? "0" : "") << seconds;
    gametimer.setString(timeStream.str());
}

// Full game reset — called after a win/death when pressing Enter.
void resetGame(bool& win, bool& menu, bool& timetogo, bool& leader,
               bool& isjumping, bool& jumpside, bool& ondabba,
               bool& istime, bool& coin1, bool& coin2, bool& coin3, bool& coin4, bool& coin5,
               bool& isname, bool& isalive, bool& isdead, bool& pipecol, bool& onpipe,
               bool& showtext, bool& setpos, bool& gamekatime,
               float& zonearea1, float& zonearea2, float& level,
               float& enemyspeed1, float& enemyspeed2, float& enemyspeed3,
               float& velocityY, float& velocityE1, float& velocityE2,
               float& velocityE3, float& velocityE4, float& winspeed,
               int& screen, int& score, int& life, int& enemycount,
               sf::Sprite& player,
               sf::Sprite& heartbox1, sf::Sprite& heartbox2, sf::Sprite& heartbox3,
               sf::Sprite& enemy1, sf::Sprite& enemy2, sf::Sprite& enemy3,
               sf::Sprite& enemy4, sf::Sprite& enemy5,
               sf::Sprite& flag2, sf::Clock& timer)
{
    win        = false;
    menu       = true;
    timetogo   = false;
    leader     = false;
    isjumping  = false;
    jumpside   = true;
    ondabba    = false;
    istime     = false;
    coin1 = coin2 = coin3 = coin4 = coin5 = true;
    isname     = true;
    isalive    = false;
    isdead     = false;
    pipecol    = false;
    onpipe     = true;
    showtext   = false;
    setpos     = true;
    gamekatime = true;
    zonearea1  = 1550.f;
    zonearea2  = 1000.f;
    level      = 840.f;
    enemyspeed1 = enemyspeed2 = enemyspeed3 = 0.07f;
    velocityY  = velocityE1 = velocityE2 = velocityE3 = velocityE4 = 1.f;
    winspeed   = 0.3f;
    screen     = 0;
    score      = 0;
    life       = 3;
    enemycount = 0;

    player.setPosition(100, level);
    heartbox1.setPosition(886, 27);
    heartbox2.setPosition(945, 27);
    heartbox3.setPosition(1003, 27);
    enemy1.setPosition(1200.f, level + 21);
    enemy2.setPosition(500.f,  level + 21);
    enemy3.setPosition(700.f,  level + 21);
    enemy4.setPosition(1020.f, level - 870);
    enemy5.setPosition(1000.f, level - 220);
    flag2.setPosition(1117, level - 398);
    timer.restart();
}


// Draws the shared HUD (score box, hearts, timer).
void drawHUD(sf::RenderWindow& window,
             sf::Sprite& backbox1, sf::Sprite& backbox2, sf::Sprite& backbox3,
             sf::Text& totalscore, sf::Text& gametimer,
             sf::Sprite& heartbox1, sf::Sprite& heartbox2, sf::Sprite& heartbox3)
{
    window.draw(backbox1);
    window.draw(totalscore);
    window.draw(backbox2);
    window.draw(heartbox1);
    window.draw(heartbox2);
    window.draw(heartbox3);
    window.draw(backbox3);
    window.draw(gametimer);
}

// Draws the staircase blocks (dabba1..dabba10).
void drawStairs(sf::RenderWindow& window,
                sf::Sprite& d1, sf::Sprite& d2, sf::Sprite& d3, sf::Sprite& d4,
                sf::Sprite& d5, sf::Sprite& d6, sf::Sprite& d7, sf::Sprite& d8,
                sf::Sprite& d9, sf::Sprite& d10, sf::Sprite& stairs)
{
    window.draw(d1);  window.draw(d2);  window.draw(d3);
    window.draw(d4);  window.draw(d5);  window.draw(d6);
    window.draw(d7);  window.draw(d8);  window.draw(d9);
    window.draw(stairs);
    window.draw(d10);
}

// Handles a single question block hit. Gives a coin if not yet collected.
void handleQuestionBlock(sf::Sprite& player, const sf::Sprite& question,
                          bool& coinCollected, bool& istime, int& score,
                          sf::Sprite& coin, sf::Vector2f coinSpawnPos,
                          sf::Music& coinSound, sf::Clock& timer)
{
    if (!player.getGlobalBounds().intersects(question.getGlobalBounds()))
        return;

    // Player bumped from below
    if (player.getPosition().y > question.getPosition().y + question.getGlobalBounds().height - 5)
    {
        player.setPosition(player.getPosition().x,
            question.getPosition().y + question.getGlobalBounds().height);

        if (coinCollected)
        {
            coinSound.play();
            coinCollected = false;
            istime  = true;
            timer.restart();
            coin.setPosition(coinSpawnPos);
            score += 10;
        }
    }
}

// Hides the coin after 1 second.
void updateCoinTimer(bool& istime, sf::Sprite& coin, const sf::Clock& timer)
{
    if (istime && timer.getElapsedTime().asSeconds() > 1.f)
    {
        coin.setPosition(-100, -100);
        istime = false;
    }
}

// Resolves all four-sided collision with a pipe sprite.
void handlePipeCollision(sf::Sprite& player, const sf::Sprite& pipe,
                          float& velocityY, bool& isjumping)
{
    if (!player.getGlobalBounds().intersects(pipe.getGlobalBounds()))
        return;

    auto pb = player.getGlobalBounds();
    auto pi = pipe.getGlobalBounds();

    // Top
    if (pb.top + pb.height <= pi.top + 5 && pb.top + pb.height > pi.top)
    {
        player.setPosition(player.getPosition().x, pi.top - pb.height);
        velocityY = 0;
        isjumping = false;
    }
    // Left
    else if (pb.left + pb.width <= pi.left + 25 && pb.left + pb.width > pi.left)
    {
        player.setPosition(pi.left - pb.width, player.getPosition().y);
    }
    // Right
    else if (pb.left >= pi.left + pi.width - 25 && pb.left < pi.left + pi.width)
    {
        player.setPosition(pi.left + pi.width, player.getPosition().y);
    }
}

// Resolves top + left-side collision for a single stair block.
void handleDabbaCollision(sf::Sprite& player, const sf::Sprite& dabba,
                           float& velocityY, bool& isjumping,
                           bool checkLeftSide = true)
{
    if (!player.getGlobalBounds().intersects(dabba.getGlobalBounds()))
        return;

    auto pb = player.getGlobalBounds();
    auto db = dabba.getGlobalBounds();

    if (checkLeftSide &&
        pb.left + pb.width >= db.left &&
        pb.left < db.left + 5 &&
        pb.top + pb.height > db.top + 5)
    {
        player.setPosition(db.left - pb.width, player.getPosition().y);
    }

    if (pb.top + pb.height <= db.top + 5 && pb.top + pb.height > db.top)
    {
        player.setPosition(player.getPosition().x, db.top - pb.height);
        velocityY = 0;
        isjumping = false;
    }
}

// Moves enemy4 (the airborne enemy on skyblocks) with gravity.
void updateAirborneEnemy(sf::Sprite& enemy, float& velocityE,
                          float& enemyspeed, float level, float gravity,
                          const sf::Sprite& sky2, const sf::Sprite& sky3,
                          const sf::RenderWindow& window)
{
    if (enemy.getPosition().x <= 0)
        return; // already dead

    if (enemy.getPosition().x <= 0 ||
        enemy.getGlobalBounds().left + enemy.getGlobalBounds().width >= window.getSize().x)
        enemyspeed = -enemyspeed;

    float newX = enemy.getPosition().x + enemyspeed;
    float newY = enemy.getPosition().y + velocityE;

    auto eb = enemy.getGlobalBounds();
    auto s2 = sky2.getGlobalBounds();
    auto s3 = sky3.getGlobalBounds();

    if (eb.intersects(s2) && eb.top + eb.height <= s2.top + 5 && velocityE > 0)
    {
        newY = s2.top - 75;
        velocityE = 0;
    }
    else if (eb.intersects(s3) && eb.top + eb.height <= s3.top + 5 && velocityE > 0)
    {
        newY = s3.top - 75;
        velocityE = 0;
    }

    if (enemy.getPosition().y >= level + 18)
        velocityE = 0;

    if (!eb.intersects(s2) && !eb.intersects(s3) && !(enemy.getPosition().y >= level + 18))
        velocityE += gravity;

    enemy.setPosition(newX, newY);
}

struct GameAssets
{
    // Textures
    sf::Texture menuTex, skyb1Tex, skyb2Tex, skyb3Tex;
    sf::Texture quesTex, coinTex, zoneTex;
    sf::Texture enemyTex, pipeTex;
    sf::Texture leftJumpTex, leftIdleTex, rightIdleTex, rightJumpTex;
    sf::Texture transDeathTex, deathTex, titleTex, leaderTex, simpleTex;
    sf::Texture winTex, winTransTex, castleTex;
    sf::Texture flag1Tex, flag2Tex, stairsTex, ekTex, backTex, heartTex;
    sf::Font    font;

    // Music
    sf::Music backMusic, jumpSound, coinSound, dieSound, winSound, smashSound, hitSound;
};

bool loadAllAssets(GameAssets& a)
{
    // -- textures --
    if (!loadTexture(a.menuTex,       "image\\menuscreen.png"))     return false;
    if (!loadTexture(a.skyb1Tex,      "image\\skyblock.png"))       return false;
    if (!loadTexture(a.skyb2Tex,      "image\\skyblock3.png"))      return false;
    if (!loadTexture(a.skyb3Tex,      "image\\skyblock2.png"))      return false;
    if (!loadTexture(a.quesTex,       "image\\questionmark.png"))   return false;
    if (!loadTexture(a.coinTex,       "image\\coin.png"))           return false;
    if (!loadTexture(a.zoneTex,       "image\\deathzone.png"))      return false;
    if (!loadTexture(a.enemyTex,      "image\\enemy1.png"))         return false;
    if (!loadTexture(a.pipeTex,       "image\\pipe.png"))           return false;
    if (!loadTexture(a.leftJumpTex,   "image\\leftjump.png"))       return false;
    if (!loadTexture(a.leftIdleTex,   "image\\leftidle.png"))       return false;
    if (!loadTexture(a.rightIdleTex,  "image\\rightidle.png"))      return false;
    if (!loadTexture(a.rightJumpTex,  "image\\rightjump.png"))      return false;
    if (!loadTexture(a.transDeathTex, "image\\deathscreentransparent.png")) return false;
    if (!loadTexture(a.deathTex,      "image\\deathscreen.png"))    return false;
    if (!loadTexture(a.titleTex,      "image\\Titlescreen.png"))    return false;
    if (!loadTexture(a.leaderTex,     "image\\leaderboard.png"))    return false;
    if (!loadTexture(a.simpleTex,     "image\\simplescreen.png"))   return false;
    if (!loadTexture(a.winTex,        "image\\winscreen.png"))      return false;
    if (!loadTexture(a.winTransTex,   "image\\winscreentrans.png")) return false;
    if (!loadTexture(a.castleTex,     "image\\castle.png"))         return false;
    if (!loadTexture(a.flag1Tex,      "image\\flag1.png"))          return false;
    if (!loadTexture(a.flag2Tex,      "image\\flag2.png"))          return false;
    if (!loadTexture(a.stairsTex,     "image\\stairs.png"))         return false;
    if (!loadTexture(a.ekTex,         "image\\ekdabba.png"))        return false;
    if (!loadTexture(a.backTex,       "image\\backbox.png"))        return false;
    if (!loadTexture(a.heartTex,      "image\\heart.png"))          return false;

    // -- font --
    if (!a.font.loadFromFile("font\\PixeloidSans.ttf"))
    {
        cerr << "Failed to load font" << endl;
        return false;
    }

    // -- music --
    if (!loadMusic(a.backMusic,  "sound\\background.ogg", 100, true)) return false;
    if (!loadMusic(a.jumpSound,  "sound\\jump.ogg",          5))      return false;
    if (!loadMusic(a.coinSound,  "sound\\coin.ogg",         50))      return false;
    if (!loadMusic(a.dieSound,   "sound\\die.ogg",           0, true)) return false;
    if (!loadMusic(a.winSound,   "sound\\win.ogg",          50))      return false;
    if (!loadMusic(a.smashSound, "sound\\smash.ogg",        30))      return false;
    if (!loadMusic(a.hitSound,   "sound\\hit.ogg",          50))      return false;

    a.backMusic.play();
    a.dieSound.play();
    return true;
}

int main()
{
    bool isjumping = false, jumpside = true, menu = true;
    bool ondabba = false, istime = false;
    bool coin1 = true, coin2 = true, coin3 = true, coin4 = true, coin5 = true;
    bool isalive = false, isdead = false;
    bool pipecol = false, onpipe = true, showtext = false;
    bool win = false, setpos = true, timetogo = false;
    bool gamekatime = true, isname = true, leader = false;

    const float gravity   =  0.002f;
    const float jumpSpeed = -0.9f;
    const float moveSpeed =  0.7f;

    float zonearea1    = 1550.f,  zonearea2  = 1000.f;
    float level        = 840.f;
    float enemyspeed1  = 0.07f,   enemyspeed2 = 0.07f, enemyspeed3 = 0.07f;
    float velocityY    = 1.f;
    float velocityE1   = 1.f, velocityE2 = 1.f, velocityE3 = 1.f, velocityE4 = 1.f;
    float winspeed     = 0.3f;

    int screen = 0, score = 0, life = 3, enemycount = 0, final = 0;
    string nameofplayer;
    string leadernames[5];

    sf::Clock deathtimer, timer, pipetimer, lifeminustimer, gametime;
    ostringstream timeStream;

    // ── window ──────────────────────────────────────────────
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Mario Game");

    // ── load all assets ─────────────────────────────────────
    GameAssets a;
    if (!loadAllAssets(a))
        return -1;

    // ── build sprites ───────────────────────────────────────

    // Menu
    sf::Sprite menupage(a.menuTex);
    fitToWindow(menupage, window);

    // Skyblocks
    sf::Sprite skyblock1(a.skyb1Tex); skyblock1.setPosition(799,  level - 219); skyblock1.setScale(0.87f, 0.9f);
    sf::Sprite skyblock2(a.skyb2Tex); skyblock2.setPosition(1220, level - 600); skyblock2.setScale(0.87f, 0.9f);
    sf::Sprite skyblock3(a.skyb3Tex); skyblock3.setPosition(850,  level - 200); skyblock3.setScale(0.87f, 0.9f);
    sf::Sprite skyblock4(a.skyb3Tex); skyblock4.setPosition(850,  level - 150); skyblock4.setScale(0.87f, 0.9f);

    // Question blocks
    sf::Sprite question1(a.quesTex); question1.setScale(1.207f, 1.207f); question1.setPosition(500,  level - 200);
    sf::Sprite question2(a.quesTex); question2.setScale(1.38f,  1.38f);  question2.setPosition(890,  level - 212.5f);
    sf::Sprite question3(a.quesTex); question3.setScale(1.38f,  1.38f);  question3.setPosition(1069, level - 212.5f);
    sf::Sprite question4(a.quesTex); question4.setScale(1.38f,  1.38f);  question4.setPosition(825,  level - 212.5f);
    sf::Sprite question5(a.quesTex); question5.setScale(1.38f,  1.38f);  question5.setPosition(1580, level - 370);

    // Coin
    sf::Sprite coin(a.coinTex); coin.setPosition(-100, -100);

    // Death zones
    sf::Sprite deathzone1(a.zoneTex); deathzone1.setPosition(zonearea1, 930); deathzone1.setScale(0.6f, 3.f);
    sf::Sprite deathzone2(a.zoneTex); deathzone2.setPosition(zonearea2, 930); deathzone2.setScale(0.6f, 3.f);

    // Enemies
    sf::Sprite enemy1(a.enemyTex); enemy1.setScale(1.3f, 1.3f); enemy1.setPosition(1200.f, level + 21);
    sf::Sprite enemy2(a.enemyTex); enemy2.setScale(1.3f, 1.3f); enemy2.setPosition(500.f,  level + 21);
    sf::Sprite enemy3(a.enemyTex); enemy3.setScale(1.3f, 1.3f); enemy3.setPosition(700.f,  level + 21);
    sf::Sprite enemy4(a.enemyTex); enemy4.setScale(1.3f, 1.3f); enemy4.setPosition(920.f,  level - 870);
    sf::Sprite enemy5(a.enemyTex); enemy5.setScale(1.3f, 1.3f); enemy5.setPosition(1000.f, level - 220);
    sf::Sprite enemy6(a.enemyTex); enemy6.setScale(1.3f, 1.3f); enemy6.setPosition(1520.f, level - 830);

    // Pipes
    sf::Sprite pipe1(a.pipeTex); pipe1.setScale(2.5f, 2.f);
    sf::Sprite pipe2(a.pipeTex); pipe2.setScale(2.5f, 2.f);

    // Player
    sf::Sprite player(a.rightIdleTex);
    player.setPosition(100, 500);
    player.setScale(1.5f, 1.5f);

    // Screens / backgrounds
    sf::Sprite transscreen(a.transDeathTex); fitToWindow(transscreen, window);
    sf::Sprite deathscreen(a.deathTex);      fitToWindow(deathscreen, window);
    sf::Sprite background(a.titleTex);       fitToWindow(background,  window);
    sf::Sprite leaderboard(a.leaderTex);     fitToWindow(leaderboard, window);
    sf::Sprite simplescreen(a.simpleTex);    fitToWindow(simplescreen, window);
    sf::Sprite winback(a.winTex);            fitToWindow(winback,     window);
    sf::Sprite winbacktrans(a.winTransTex);  fitToWindow(winbacktrans, window);

    // Castle, flags, stairs, dabba blocks
    sf::Sprite castle(a.castleTex);   castle.setScale(0.9f,  0.9f);  castle.setPosition(1450, level - 369);
    sf::Sprite flag1(a.flag1Tex);     flag1.setPosition(1100, level - 420);
    sf::Sprite flag2(a.flag2Tex);     flag2.setPosition(1117, level - 398); flag2.setScale(0.2f, 0.2f);
    sf::Sprite stairs(a.stairsTex);   stairs.setScale(0.9f,  0.9f);  stairs.setPosition(300, level - 314.25f);

    sf::Sprite dabba1(a.ekTex);  dabba1.setScale(1.5f,1.5f);  dabba1.setPosition(300,  level + 45);
    sf::Sprite dabba2(a.ekTex);  dabba2.setScale(1.5f,1.5f);  dabba2.setPosition(352,  level - 7);
    sf::Sprite dabba3(a.ekTex);  dabba3.setScale(1.5f,1.5f);  dabba3.setPosition(402,  level - 60);
    sf::Sprite dabba4(a.ekTex);  dabba4.setScale(1.5f,1.5f);  dabba4.setPosition(452,  level - 111);
    sf::Sprite dabba5(a.ekTex);  dabba5.setScale(1.5f,1.5f);  dabba5.setPosition(504,  level - 163);
    sf::Sprite dabba6(a.ekTex);  dabba6.setScale(1.5f,1.5f);  dabba6.setPosition(556,  level - 213);
    sf::Sprite dabba7(a.ekTex);  dabba7.setScale(1.5f,1.5f);  dabba7.setPosition(606,  level - 265);
    sf::Sprite dabba8(a.ekTex);  dabba8.setScale(1.5f,1.5f);  dabba8.setPosition(659,  level - 314);
    sf::Sprite dabba9(a.ekTex);  dabba9.setScale(1.5f,1.5f);  dabba9.setPosition(712,  level - 314);
    sf::Sprite dabba10(a.ekTex); dabba10.setScale(1.5f,1.5f); dabba10.setPosition(1093, level + 45);

    // HUD sprites
    sf::Sprite backbox1(a.backTex); backbox1.setPosition(20,   20); backbox1.setScale(0.25f, 0.5f);
    sf::Sprite backbox2(a.backTex); backbox2.setPosition(870,  20); backbox2.setScale(0.25f, 0.5f);
    sf::Sprite backbox3(a.backTex); backbox3.setPosition(1700, 20); backbox3.setScale(0.25f, 0.5f);

    sf::Sprite heartbox1(a.heartTex); heartbox1.setScale(0.31f,0.31f); heartbox1.setPosition(886, 27);
    sf::Sprite heartbox2(a.heartTex); heartbox2.setScale(0.31f,0.31f); heartbox2.setPosition(945, 27);
    sf::Sprite heartbox3(a.heartTex); heartbox3.setScale(0.31f,0.31f); heartbox3.setPosition(1003,27);

    // ── HUD text ────────────────────────────────────────────
    auto makeText = [&](sf::Color color, int size, float x, float y) -> sf::Text {
        sf::Text t;
        t.setFont(a.font);
        t.setFillColor(color);
        t.setCharacterSize(size);
        t.setPosition(x, y);
        return t;
    };

    sf::Text totalscore   = makeText(sf::Color::White,  30,   32,   27);
    sf::Text noliveleft   = makeText(sf::Color::White,  30,  925,   27); noliveleft.setString("DEAD");
    sf::Text lifeminustext1 = makeText(sf::Color::Red,  30, 1565, level + 100); lifeminustext1.setString("1 life lost");
    sf::Text lifeminustext2 = makeText(sf::Color::Red,  30, zonearea2 + 25, level + 100); lifeminustext2.setString("1 life lost");
    sf::Text maintime     = makeText(sf::Color::White,  70,  730,  330);
    sf::Text welldone     = makeText(sf::Color::Yellow, 70,  570,  330); welldone.setString("\n\nTHANKS FOR PLAYING");
    sf::Text enter        = makeText(sf::Color::White,  30,  720,  630); enter.setString("\n\nPress ENTER to continue!!!");
    sf::Text gametimer    = makeText(sf::Color::White,  30, 1715,   30);
    sf::Text leadert1     = makeText(sf::Color::Yellow, 30,  850,  350);
    sf::Text leadert2     = makeText(sf::Color::Cyan,   30,  850,  430);
    sf::Text leadert3     = makeText(sf::Color::Blue,   30,  850,  510);
    sf::Text leadert4     = makeText(sf::Color::White,  30,  850,  590);
    sf::Text leadert5     = makeText(sf::Color::White,  30,  850,  670);

    // ── main game loop ───────────────────────────────────────
    while (window.isOpen())
    {
        // ── event handling ──────────────────────────────────
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                menu = !menu;
        }

        // ── timer display (always updated) ──────────────────
        updateTimerDisplay(gametime, gametimer, timeStream);

        // ════════════════════════════════════════════════════
        //  GAME STATE BRANCHES
        // ════════════════════════════════════════════════════

        if (menu)
        {
            // ── Menu screen ─────────────────────────────────
            a.backMusic.setVolume(100);
            a.dieSound.setVolume(0);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::P))
            {
                menu = false; leader = false;
                player.setPosition(100, level - 400);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
                window.close();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
            {
                resetGame(win, menu, timetogo, leader,
                          isjumping, jumpside, ondabba, istime,
                          coin1, coin2, coin3, coin4, coin5, isname,
                          isalive, isdead, pipecol, onpipe, showtext,
                          setpos, gamekatime, zonearea1, zonearea2, level,
                          enemyspeed1, enemyspeed2, enemyspeed3,
                          velocityY, velocityE1, velocityE2, velocityE3, velocityE4,
                          winspeed, screen, score, life, enemycount,
                          player, heartbox1, heartbox2, heartbox3,
                          enemy1, enemy2, enemy3, enemy4, enemy5, flag2, timer);
                menu = false;
                continue;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::L))
            {
                leader = true; menu = false;
            }

            // Player still moves in menu (original behaviour)
            handleJumpInput(player, isjumping, jumpside, velocityY, jumpSpeed,
                            a.jumpSound, a.leftJumpTex, a.rightJumpTex);
            if (isjumping) velocityY += gravity;
            player.move(0, velocityY);
            if (player.getPosition().y >= level) { player.setPosition(player.getPosition().x, level); isjumping = false; }
            updateIdleTexture(player, isjumping, jumpside, a.leftIdleTex, a.rightIdleTex);
            handleHorizontalMovement(player, jumpside, moveSpeed,
                                     a.leftJumpTex, a.rightJumpTex, window);
        }
        else if (leader)
        {
            // ── Leaderboard screen ──────────────────────────
            leaderboardscore("score.txt", leadernames);
            leadert1.setString(leadernames[0]);
            leadert2.setString(leadernames[1]);
            leadert3.setString(leadernames[2]);
            leadert4.setString(leadernames[3]);
            leadert5.setString(leadernames[4]);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
            {
                leader = false; menu = true;
            }
        }
        else if (win)
        {
            // ── Win animation ───────────────────────────────
            if (gamekatime)
            {
                maintime.setString(timeStream.str());
                int secs = static_cast<int>(gametime.getElapsedTime().asSeconds());
                final = secs % 60 + (secs / 60 * 60);
                updateLeaderboard("score.txt", nameofplayer, final);
                gamekatime = false;
            }
            if (player.getPosition().x > 0)
            {
                player.setTexture(a.rightIdleTex);
                if (setpos) { player.setPosition(1000, level); setpos = false; }

                if (player.getPosition().x == 1000 && flag2.getPosition().y <= level - 30)
                    flag2.setPosition(flag2.getPosition().x, flag2.getPosition().y + winspeed);

                if (flag2.getPosition().y >= level - 45 && player.getPosition().x < 1600)
                {
                    player.setTexture(a.rightJumpTex);
                    player.move(winspeed - 0.1f, 0);
                    timer.restart();
                }
                if (player.getPosition().x >= 1600 && timer.getElapsedTime().asSeconds() > 1.5f)
                {
                    win = false; timetogo = true;
                    player.setPosition(-100, -100);
                }
            }
        }
        else if (isdead)
        {
            // ── Dead screen ──────────────────────────────────
            a.backMusic.setVolume(0);
            a.dieSound.setVolume(50);
        }
        else if (screen == 0)
        {
            // ── SCREEN 0 ────────────────────────────────────
            if (screen == 0 && enemycount == 0 && isname)
            {
                nameofplayer = askname(a.font, " ");
                isname = false;
            }
            if (gamekatime) { gametime.restart(); gamekatime = false; }

            // Advance to next screen
            if (player.getPosition().x >= 1820 && enemycount >= 1 && score >= 40)
            {
                player.setPosition(100, level); screen = 1; continue;
            }

            // Pipe collision
            pipe1.setPosition(1400, level - 37);
            {
                if (player.getGlobalBounds().intersects(pipe1.getGlobalBounds()))
                {
                    auto pb = player.getGlobalBounds();
                    auto pi = pipe1.getGlobalBounds();
                    if (!pipecol)
                    {
                        if (pb.top + pb.height <= pi.top + 1)
                        {
                            player.setPosition(player.getPosition().x, pi.top - pb.height);
                            velocityY = 0; isjumping = false;
                        }
                        pipecol = true; pipetimer.restart();
                    }
                    if (pb.left < pi.left - 25)
                        player.setPosition(pi.left - pb.width, player.getPosition().y);
                    else if (pb.left + pb.width > pi.left + pi.width + 30)
                        player.setPosition(pi.left + pi.width, player.getPosition().y);
                    onpipe = true;
                }
                if (!player.getGlobalBounds().intersects(pipe1.getGlobalBounds()))
                    onpipe = false;
                if (pipecol && pipetimer.getElapsedTime().asSeconds() > 0.2f)
                    pipecol = false;
            }

            totalscore.setString("Score: " + to_string(score));

            // Enemy 1
            if (enemy1.getPosition().x <= 0 ||
                enemy1.getGlobalBounds().intersects(pipe1.getGlobalBounds()))
                enemyspeed1 = -enemyspeed1;
            enemy1.setPosition(enemy1.getPosition().x + enemyspeed1, level + 21);
            handleEnemyCollision(player, enemy1, isjumping, isalive, life, score,
                                  enemycount, deathtimer, a.smashSound, a.hitSound, level);
            updateInvincibility(isalive, deathtimer);

            // Coins
            handleQuestionBlock(player, question1, coin1, istime, score, coin,
                                 { 510.f, level - 270 }, a.coinSound, timer);
            updateCoinTimer(istime, coin, timer);
            handleQuestionBlock(player, question2, coin2, istime, score, coin,
                                 { 904.f, level - 283 }, a.coinSound, timer);
            updateCoinTimer(istime, coin, timer);
            handleQuestionBlock(player, question3, coin3, istime, score, coin,
                                 { 1084.f, level - 283 }, a.coinSound, timer);
            updateCoinTimer(istime, coin, timer);

            // Skyblock1
            resolveBlockCollision(player, velocityY, isjumping, skyblock1);

            // Basic movement
            handleJumpInput(player, isjumping, jumpside, velocityY, jumpSpeed,
                            a.jumpSound, a.leftJumpTex, a.rightJumpTex);
            applyGravityAndGround(player, velocityY, isjumping,
                                   isjumping || !onpipe, level, gravity);
            updateIdleTexture(player, isjumping, jumpside, a.leftIdleTex, a.rightIdleTex);
            handleHorizontalMovement(player, jumpside, moveSpeed,
                                     a.leftJumpTex, a.rightJumpTex, window);

            updateHearts(life, isdead, heartbox1, heartbox2, heartbox3);
        }
        else if (screen == 1)
        {
            // ── SCREEN 1 ────────────────────────────────────
            gamekatime = true;
            if (player.getPosition().x >= 1820 && enemycount >= 3 && score >= 70)
            {
                player.setPosition(50, level); screen = 2; continue;
            }

            deathzone1.setPosition(zonearea1, 930);
            totalscore.setString("Score: " + to_string(score));

            // Coin 4 (question4 — special collision)
            if (player.getGlobalBounds().intersects(question4.getGlobalBounds()))
            {
                auto pb = player.getGlobalBounds();
                auto qb = question4.getGlobalBounds();
                if (pb.top > qb.top + qb.height - 5)
                    player.setPosition(pb.left, qb.top + qb.height);
                else if (pb.top + pb.height <= qb.top + 5)
                    player.setPosition(pb.left, qb.top - pb.height);
                else if (pb.left < qb.left && pb.left + pb.width > qb.left)
                    player.setPosition(qb.left - pb.width, pb.top);
                else if (pb.left + pb.width > qb.left + qb.width && pb.left < qb.left + qb.width)
                    player.setPosition(qb.left + qb.width, pb.top);

                if (coin4)
                {
                    a.coinSound.play(); coin4 = false; istime = true; timer.restart();
                    coin.setPosition(839.f, level - 283); score += 10;
                }
            }
            updateCoinTimer(istime, coin, timer);

            // Pipes
            pipe1.setPosition(300, level - 37);
            handlePipeCollision(player, pipe1, velocityY, isjumping);
            pipe2.setPosition(1200, level - 37);
            handlePipeCollision(player, pipe2, velocityY, isjumping);

            // Enemy 2
            if (enemy2.getGlobalBounds().intersects(pipe1.getGlobalBounds()) ||
                enemy2.getGlobalBounds().intersects(pipe2.getGlobalBounds()))
                enemyspeed1 = -enemyspeed1;
            enemy2.setPosition(enemy2.getPosition().x + enemyspeed1, level + 21);
            if (handleEnemyCollision(player, enemy2, isjumping, isalive, life, score,
                                      enemycount, deathtimer, a.smashSound, a.hitSound, level))
                enemyspeed1 = 0;
            updateInvincibility(isalive, deathtimer);

            // Enemy 3
            if (enemy3.getGlobalBounds().intersects(pipe1.getGlobalBounds()) ||
                enemy3.getGlobalBounds().intersects(pipe2.getGlobalBounds()))
                enemyspeed2 = -enemyspeed2;
            enemy3.setPosition(enemy3.getPosition().x + enemyspeed2, level + 21);
            handleEnemyCollision(player, enemy3, isjumping, isalive, life, score,
                                  enemycount, deathtimer, a.smashSound, a.hitSound, level);
            updateInvincibility(isalive, deathtimer);

            // Movement + death zone
            handleJumpInput(player, isjumping, jumpside, velocityY, jumpSpeed,
                            a.jumpSound, a.leftJumpTex, a.rightJumpTex);
            if (isjumping || !onpipe) velocityY += gravity;
            player.move(0, velocityY);

            if (player.getPosition().y >= level &&
                !(player.getPosition().x >= zonearea1 && player.getPosition().x <= zonearea1 + 150))
            {
                player.setPosition(player.getPosition().x, level);
                isjumping = false;
            }
            else if (player.getPosition().y > level + 100)
            {
                player.setPosition(100, level - 200);
                life--; showtext = true; lifeminustimer.restart();
            }

            if (showtext && lifeminustimer.getElapsedTime().asSeconds() > 1.5f)
                showtext = false;

            updateIdleTexture(player, isjumping, jumpside, a.leftIdleTex, a.rightIdleTex);
            handleHorizontalMovement(player, jumpside, moveSpeed,
                                     a.leftJumpTex, a.rightJumpTex, window);
            updateHearts(life, isdead, heartbox1, heartbox2, heartbox3);
        }
        else if (screen == 2)
        {
            // ── SCREEN 2 ────────────────────────────────────
            if (player.getPosition().x >= 1820 && enemycount >= 4 && score >= 80)
            {
                player.setPosition(100, level); screen = 3; continue;
            }

            deathzone2.setPosition(zonearea2, 930);
            totalscore.setString("Score: " + to_string(score));

            // Enemy 4 (airborne)
            updateAirborneEnemy(enemy4, velocityE1, enemyspeed1, level, gravity,
                                 skyblock2, skyblock3, window);
            handleEnemyCollision(player, enemy4, isjumping, isalive, life, score,
                                  enemycount, deathtimer, a.smashSound, a.hitSound, level);
            updateInvincibility(isalive, deathtimer);

            // Coin 5
            handleQuestionBlock(player, question5, coin5, istime, score, coin,
                                 { 1595.f, level - 450 }, a.coinSound, timer);
            updateCoinTimer(istime, coin, timer);

            // Pipe 2
            pipe2.setPosition(1550, level - 37);
            handlePipeCollision(player, pipe2, velocityY, isjumping);

            // Skyblock3 collision
            resolveBlockCollision(player, velocityY, isjumping, skyblock3);

            // Death-zone movement
            {
                zonearea1 = 1000.f;
                deathzone1.setPosition(zonearea1, 930);

                handleJumpInput(player, isjumping, jumpside, velocityY, jumpSpeed,
                                a.jumpSound, a.leftJumpTex, a.rightJumpTex);
                if (isjumping || !onpipe) velocityY += gravity;
                updateIdleTexture(player, isjumping, jumpside, a.leftIdleTex, a.rightIdleTex);
                player.move(0, velocityY);

                if (player.getPosition().y >= level &&
                    !(player.getPosition().x > zonearea2 && player.getPosition().x <= zonearea2 + 150))
                {
                    player.setPosition(player.getPosition().x, level);
                    isjumping = false;
                }
                else if (player.getPosition().y > level + 100)
                {
                    player.setPosition(100, level - 200);
                    life--; showtext = true; lifeminustimer.restart();
                }
                if (showtext && lifeminustimer.getElapsedTime().asSeconds() > 1.5f)
                    showtext = false;

                handleHorizontalMovement(player, jumpside, moveSpeed,
                                         a.leftJumpTex, a.rightJumpTex, window);
            }
            updateHearts(life, isdead, heartbox1, heartbox2, heartbox3);
        }
        else if (screen == 3)
        {
            // ── SCREEN 3 ────────────────────────────────────
            if (player.getPosition().x >= 1820 && enemycount >= 5 && score >= 90)
            {
                player.setPosition(100, level); screen = 4; continue;
            }

            // Skyblock4
            resolveBlockCollision(player, velocityY, isjumping, skyblock4);

            // Pipe 1
            pipe1.setPosition(300, level - 37);
            {
                auto pb = player.getGlobalBounds();
                auto pi = pipe1.getGlobalBounds();
                if (player.getGlobalBounds().intersects(pipe1.getGlobalBounds()))
                {
                    if (pb.top + pb.height <= pi.top + 1)
                    {
                        player.setPosition(pb.left, pi.top - pb.height);
                        velocityY = 0; isjumping = false;
                    }
                    pipetimer.restart();
                    if (pb.left < pi.left - 25)
                        player.setPosition(pi.left - pb.width, pb.top);
                    else if (pb.left + pb.width > pi.left + pi.width + 30)
                        player.setPosition(pi.left + pi.width, pb.top);
                    onpipe = true;
                }
                if (!player.getGlobalBounds().intersects(pipe1.getGlobalBounds()))
                    onpipe = false;
            }

            // Enemy 5 (static skyblock enemy)
            handleEnemyCollision(player, enemy5, isjumping, isalive, life, score,
                                  enemycount, deathtimer, a.smashSound, a.hitSound, level);
            updateInvincibility(isalive, deathtimer);

            totalscore.setString("Score: " + to_string(score));

            handleJumpInput(player, isjumping, jumpside, velocityY, jumpSpeed,
                            a.jumpSound, a.leftJumpTex, a.rightJumpTex);
            if (isjumping || !onpipe) velocityY += gravity;
            updateIdleTexture(player, isjumping, jumpside, a.leftIdleTex, a.rightIdleTex);
            applyGravityAndGround(player, velocityY, isjumping, false, level, gravity);
            handleHorizontalMovement(player, jumpside, moveSpeed,
                                     a.leftJumpTex, a.rightJumpTex, window);
            updateHearts(life, isdead, heartbox1, heartbox2, heartbox3);
        }
        else if (screen == 4)
        {
            // ── SCREEN 4 (final level) ───────────────────────
            totalscore.setString("Score: " + to_string(score));

            // All 10 stair blocks
            handleDabbaCollision(player, dabba1,  velocityY, isjumping);
            handleDabbaCollision(player, dabba2,  velocityY, isjumping);
            handleDabbaCollision(player, dabba3,  velocityY, isjumping);
            handleDabbaCollision(player, dabba4,  velocityY, isjumping);
            handleDabbaCollision(player, dabba5,  velocityY, isjumping);
            handleDabbaCollision(player, dabba6,  velocityY, isjumping);
            handleDabbaCollision(player, dabba7,  velocityY, isjumping);
            handleDabbaCollision(player, dabba8,  velocityY, isjumping);
            handleDabbaCollision(player, dabba9,  velocityY, isjumping, false); // no left check
            handleDabbaCollision(player, dabba10, velocityY, isjumping);

            // Staircase right-side block
            if (player.getGlobalBounds().intersects(stairs.getGlobalBounds()))
            {
                auto pb = player.getGlobalBounds();
                auto sb = stairs.getGlobalBounds();
                if (pb.left < sb.left + sb.width &&
                    pb.left + pb.width > sb.left + sb.width)
                    player.setPosition(sb.left + sb.width, player.getPosition().y);
            }

            // Flag → triggers win sequence
            if (player.getGlobalBounds().left + player.getGlobalBounds().width >=
                flag1.getGlobalBounds().left + 10)
            {
                player.setPosition(1300, level);
                win = true;
                a.backMusic.setVolume(30);
                a.winSound.play();
            }

            // Movement
            handleJumpInput(player, isjumping, jumpside, velocityY, jumpSpeed,
                            a.jumpSound, a.leftJumpTex, a.rightJumpTex);
            if (isjumping || !ondabba) velocityY += gravity;
            updateIdleTexture(player, isjumping, jumpside, a.leftIdleTex, a.rightIdleTex);
            player.move(0, velocityY);
            if (player.getPosition().y >= level)
            {
                player.setPosition(player.getPosition().x, level);
                isjumping = false;
            }
            handleHorizontalMovement(player, jumpside, moveSpeed,
                                     a.leftJumpTex, a.rightJumpTex, window);
        }

        // ════════════════════════════════════════════════════
        //  DRAWING
        // ════════════════════════════════════════════════════
        window.clear();

        if (menu)
        {
            window.draw(menupage);
            window.draw(player);
        }
        else if (leader)
        {
            window.draw(leaderboard);
            window.draw(leadert1); window.draw(leadert2); window.draw(leadert3);
            window.draw(leadert4); window.draw(leadert5);
        }
        else if (win)
        {
            window.draw(winback);
            drawHUD(window, backbox1, backbox2, backbox3, totalscore, gametimer,
                    heartbox1, heartbox2, heartbox3);
            window.draw(castle);
            drawStairs(window, dabba1, dabba2, dabba3, dabba4, dabba5,
                       dabba6, dabba7, dabba8, dabba9, dabba10, stairs);
            window.draw(flag1); window.draw(flag2);
            window.draw(player);
        }
        else if (timetogo)
        {
            window.draw(winback);
            drawHUD(window, backbox1, backbox2, backbox3, totalscore, gametimer,
                    heartbox1, heartbox2, heartbox3);
            window.draw(castle);
            drawStairs(window, dabba1, dabba2, dabba3, dabba4, dabba5,
                       dabba6, dabba7, dabba8, dabba9, dabba10, stairs);
            window.draw(flag1); window.draw(flag2);
            window.draw(winbacktrans);
            window.draw(maintime); window.draw(welldone); window.draw(enter);

            // Reset on Enter
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
            {
                resetGame(win, menu, timetogo, leader,
                          isjumping, jumpside, ondabba, istime,
                          coin1, coin2, coin3, coin4, coin5, isname,
                          isalive, isdead, pipecol, onpipe, showtext,
                          setpos, gamekatime, zonearea1, zonearea2, level,
                          enemyspeed1, enemyspeed2, enemyspeed3,
                          velocityY, velocityE1, velocityE2, velocityE3, velocityE4,
                          winspeed, screen, score, life, enemycount,
                          player, heartbox1, heartbox2, heartbox3,
                          enemy1, enemy2, enemy3, enemy4, enemy5, flag2, timer);
            }
        }
        else if (isdead)
        {
            window.draw(deathscreen);
            window.draw(backbox1); window.draw(totalscore);
            window.draw(heartbox1); window.draw(heartbox2); window.draw(heartbox3);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter))
            {
                resetGame(win, menu, timetogo, leader,
                          isjumping, jumpside, ondabba, istime,
                          coin1, coin2, coin3, coin4, coin5, isname,
                          isalive, isdead, pipecol, onpipe, showtext,
                          setpos, gamekatime, zonearea1, zonearea2, level,
                          enemyspeed1, enemyspeed2, enemyspeed3,
                          velocityY, velocityE1, velocityE2, velocityE3, velocityE4,
                          winspeed, screen, score, life, enemycount,
                          player, heartbox1, heartbox2, heartbox3,
                          enemy1, enemy2, enemy3, enemy4, enemy5, flag2, timer);
            }
        }
        else if (screen == 0)
        {
            window.draw(background);
            window.draw(pipe1);   window.draw(enemy1);
            window.draw(skyblock1);
            window.draw(question1); window.draw(question2); window.draw(question3);
            drawHUD(window, backbox1, backbox2, backbox3, totalscore, gametimer,
                    heartbox1, heartbox2, heartbox3);
            if (istime) window.draw(coin);
            window.draw(player);
        }
        else if (screen == 1)
        {
            window.draw(simplescreen);
            window.draw(deathzone1);
            window.draw(pipe1); window.draw(pipe2);
            window.draw(enemy2); window.draw(enemy3);
            window.draw(question4); window.draw(coin);
            drawHUD(window, backbox1, backbox2, backbox3, totalscore, gametimer,
                    heartbox1, heartbox2, heartbox3);
            if (showtext) window.draw(lifeminustext1);
            window.draw(player);
        }
        else if (screen == 2)
        {
            window.draw(simplescreen);
            window.draw(skyblock2); window.draw(skyblock3);
            window.draw(enemy4);
            window.draw(deathzone2);
            window.draw(pipe2); window.draw(question5); window.draw(coin);
            drawHUD(window, backbox1, backbox2, backbox3, totalscore, gametimer,
                    heartbox1, heartbox2, heartbox3);
            if (showtext) window.draw(lifeminustext2);
            window.draw(player);
        }
        else if (screen == 3)
        {
            window.draw(simplescreen);
            window.draw(skyblock4);
            window.draw(enemy5); window.draw(pipe1);
            drawHUD(window, backbox1, backbox2, backbox3, totalscore, gametimer,
                    heartbox1, heartbox2, heartbox3);
            window.draw(player);
        }
        else if (screen == 4)
        {
            window.draw(simplescreen);
            window.draw(castle);
            drawStairs(window, dabba1, dabba2, dabba3, dabba4, dabba5,
                       dabba6, dabba7, dabba8, dabba9, dabba10, stairs);
            window.draw(flag1); window.draw(flag2);
            drawHUD(window, backbox1, backbox2, backbox3, totalscore, gametimer,
                    heartbox1, heartbox2, heartbox3);
            window.draw(player);
        }

        window.display();
    }

    return 0;
}
