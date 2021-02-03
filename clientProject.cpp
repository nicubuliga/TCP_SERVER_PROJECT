#include <SFML/Graphics.hpp>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <bits/stdc++.h>
#include <string>
#include <chrono> 
#include <time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

using namespace std::chrono; 

#define DELETE 8
#define ENTER 27
#define FIONREAD 0x541B
#define F_SETFL 4
#define F_GETFL 3
#define O_NONBLOCK 04000
#define POLLIN 0x001

using namespace std;

typedef struct thData{
 int idThread; //id-ul thread-ului tinut in evidenta de acest program
 int cl; //descriptorul intors de accept
}thData;

pthread_t th[10]; //Identificatorii thread-urilor care se vor crea

struct pollfd fds[1];

bool connected = false;
bool infoDisplay = false;
bool stopProgram = false;

int speed, realSpeed = -1;

string weather, temp, sport;

int sd;			// descriptorul de socket
int i = 0;
struct sockaddr_in server;	// structura folosita pentru conectare

/* portul de conectare la server*/
int port;

//variabile boolene pentru indeplinirea unor conditii
bool hover1 = false, hover2 = false, hoverSubmit1 = false, hoverSubmit2 = false;
bool activeBox1 = false, activeBox2 = false;
bool waitingForStreet = false, waitingForLoc1 = false, waitingForLoc2 = false;

//shortest path variables
int loc1, loc2;

int point1, point2, indexStreet;

bool escape = false;

float x, y;

vector< pair<float, float> > locations = {
    {250.f, 80.f}, {370.f, 80.f}, {470.f, 80.f}, {670.f, 80.f}, {850.f, 80.f},
    {420.f, 200.f}, {470.f, 200.f}, {610.f, 200.f}, 
    {370.f, 300.f}, {420.f, 300.f}, {470.f, 300.f}, {560.f, 300.f},
    {250.f, 400.f}, {420.f, 400.f}, {470.f, 400.f}, {560.f, 400.f}, {670.f, 400.f},
    {250.f, 470.f}, {470.f, 470.f},
    {250.f, 525.f}, {420.f, 525.f}, {850.f, 525.f}
};

bool matrix[22][22] = {
   //0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20 21
    {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //0
    {0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //1
    {0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //2
    {0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, //3
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, //4
    {0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //5
    {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //6
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //7
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //8
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, //9
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0}, //10
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0}, //11
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0}, //12
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0}, //13
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0}, //14
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, //15
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //16
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0}, //17
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //18
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0}, //19
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, //20
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}  //21
};

vector<int> path(22);
bool showPath = false;
int source;
int nr = 0;

int cost[22][22];

vector< pair< pair< float, float>, pair< float, float> > > streets;
vector< pair< int, int> > streetsIndx;
int streetColor[22][22];

int currentPage = 1;

string loginInput = "", passwordInput = "", loginRegister = "", passwordRegister = "";

static void *waitUpdates(void *);

void connectToServer(int argc, char *argv[])
{
     /* exista toate argumentele in linia de comanda? */
    if (argc != 3)
    {
        printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        exit(1);
    }

    /* stabilim portul */
    port = atoi (argv[2]);

    /* cream socketul */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("Eroare la socket().\n");
        exit(1);
    }

    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    /* familia socket-ului */
    server.sin_family = AF_INET;
    /* adresa IP a serverului */
    server.sin_addr.s_addr = inet_addr(argv[1]);
    /* portul de conectare */
    server.sin_port = htons (port);

    /* ne conectam la server */
    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
        perror ("Eroare la connect().\n");
        exit(1);
    }
}

void drawLoginButton(sf::RenderWindow &window, bool hover) 
{
    sf::Texture loginTexture;
    if (!loginTexture.loadFromFile("buttonLogin.png"))
    {
        printf("Eroare la buton login\n");
    }

    sf::Sprite loginSprite;
    loginSprite.setTexture(loginTexture);
    loginTexture.setSmooth(true);
    loginSprite.setPosition(sf::Vector2f(378.5f, 100.f));
    loginSprite.setScale({1.5, 1.5});

    if(hover) 
        loginSprite.setColor(sf::Color(255, 255, 255, 128));
    
    window.draw(loginSprite);
}

void drawRegisterButton(sf::RenderWindow &window, bool hover) 
{
    sf::Texture registerTexture;
    if (!registerTexture.loadFromFile("buttonRegister.png"))
    {
        printf("Eroare la buton register\n");
    }

    sf::Sprite registerSprite;
    registerSprite.setTexture(registerTexture);
    registerTexture.setSmooth(true);
    registerSprite.setPosition(sf::Vector2f(342.5f, 200.f));
    registerSprite.setScale({1.5, 1.5});

    if(hover) 
        registerSprite.setColor(sf::Color(255, 255, 255, 128));
    
    window.draw(registerSprite);
}

void drawFirstPage(sf::RenderWindow &window, bool hoverLogin, bool hoverRegister)
{
    sf::Texture texture;
    if (!texture.loadFromFile("image1.png"))
    {
        printf("Eroare la background\n");
    }

    sf::Sprite sprite;
    sprite.setTexture(texture);
    sprite.setTextureRect(sf::IntRect(0, 0, 1000, 600));
    // sprite.setColor(sf::Color(56, 93, 56));
    // texture.setSmooth(true);
    //sprite.setTextureRect(sf::IntRect(10, 10, 32, 32));
    // sf::Font arial;
    // arial.loadFromFile("Arial.ttf");
    window.draw(sprite);
    drawLoginButton(window, hoverLogin);
    drawRegisterButton(window, hoverRegister);

}

void drawTextLogin(sf::RenderWindow &window)
{
    sf::Texture textLoginTexture;
    if (!textLoginTexture.loadFromFile("textLOGIN.png"))
    {
        printf("Eroare la buton login\n");
    }

    sf::Sprite textLoginSprite;
    textLoginSprite.setTexture(textLoginTexture);
    textLoginTexture.setSmooth(true);
    textLoginSprite.setPosition(sf::Vector2f(392.f, 30.f));
    textLoginSprite.setScale({0.5, 0.5});
    
    window.draw(textLoginSprite);
}

void drawTextRegister(sf::RenderWindow &window)
{
    sf::Texture textLoginTexture;
    if (!textLoginTexture.loadFromFile("textREGISTER.png"))
    {
        printf("Eroare la buton register\n");
    }

    sf::Sprite textLoginSprite;
    textLoginSprite.setTexture(textLoginTexture);
    textLoginTexture.setSmooth(true);
    textLoginSprite.setPosition(sf::Vector2f(356.5f, 50.f));
    textLoginSprite.setScale({0.5, 0.5});
    
    window.draw(textLoginSprite);
}

void drawTextBoxes(sf::RenderWindow &window)
{
    //~~~~~~~~~~BOX1~~~~~~~~~~~~~~~~~~~~~~
    sf::Texture box1;
    if (!box1.loadFromFile("box1.png"))
    {
        printf("Eroare la buton login\n");
    }

    sf::Sprite box1Sprite;
    box1Sprite.setTexture(box1);
    box1.setSmooth(true);
    box1Sprite.setPosition(sf::Vector2f(350.f, 130.f));
    box1Sprite.setScale({6, 1.5});
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    //~~~~~~~~~~BOX2~~~~~~~~~~~~~~~~~~~~~~
    sf::Texture box2;
    if (!box2.loadFromFile("box1.png"))
    {
        printf("Eroare la buton login\n");
    }

    sf::Sprite box2Sprite;
    box2Sprite.setTexture(box2);
    box2.setSmooth(true);
    box2Sprite.setPosition(sf::Vector2f(350.f, 180.f));
    box2Sprite.setScale({6, 1.5});
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    
    window.draw(box1Sprite);
    window.draw(box2Sprite);
}

void displayTextLogin(sf::RenderWindow &window, float xText, float yText)
{
    sf::Text textBox;
    textBox.setCharacterSize(25);
    textBox.setFillColor(sf::Color::Black);
    sf::Font arial;
    arial.loadFromFile("Arial.ttf");

    textBox.setFont(arial);
    if(currentPage == 2) {
        if(activeBox1)
            textBox.setString(loginInput + '_');
        else
            textBox.setString(loginInput);
    } else 
    if(currentPage == 3) {
        if(activeBox1)
            textBox.setString(loginRegister + '_');
        else
            textBox.setString(loginRegister);
    }
    textBox.setPosition({xText, yText});
    window.draw(textBox);
}

void displayPasswordLogin(sf::RenderWindow &window, float xText, float yText)
{
    sf::Text textBox;
    textBox.setCharacterSize(25);
    textBox.setFillColor(sf::Color::Black);
    sf::Font arial;
    arial.loadFromFile("Arial.ttf");

    string auxStr = "";
    if(currentPage == 2) {
        for(int i = 0; i < passwordInput.length(); ++i)
            auxStr += "*";
    } else 
    if(currentPage == 3) {
        for(int i = 0; i < passwordRegister.length(); ++i)
            auxStr += "*";
    }
    textBox.setFont(arial);
    if(activeBox2)
        textBox.setString(auxStr + '_');
    else
        textBox.setString(auxStr);
    textBox.setPosition({xText, yText});
    window.draw(textBox);
}

void drawSubmit(sf::RenderWindow &window, bool hover)
{
    sf::Texture submitTexture;
    if (!submitTexture.loadFromFile("submit.png"))
    {
        printf("Eroare la buton submit\n");
    }

    sf::Sprite submitSprite;
    submitSprite.setTexture(submitTexture);
    submitTexture.setSmooth(true);
    submitSprite.setPosition(sf::Vector2f(425.f, 250.f));
    //submitSprite.setScale({1.5, 1.5});

    if(hover) 
        submitSprite.setColor(sf::Color(255, 255, 255, 128));
    
    window.draw(submitSprite);
}

void drawBack(sf::RenderWindow &window)
{
    sf::Texture backTexture;
    if (!backTexture.loadFromFile("back.png"))
    {
        printf("Eroare la buton back\n");
    }

    sf::Sprite backSprite;
    backSprite.setTexture(backTexture);
    backTexture.setSmooth(true);
    backSprite.setPosition(sf::Vector2f(25.f, 25.f));
    
    window.draw(backSprite);
}

void drawSecondPage(sf::RenderWindow &window)
{
    sf::Texture texture;
    if (!texture.loadFromFile("image1.png"))
    {
        printf("Eroare la background\n");
    }

    sf::Sprite sprite;
    sprite.setTexture(texture);
    sprite.setTextureRect(sf::IntRect(0, 0, 1000, 600));

    window.draw(sprite);
    
    drawTextLogin(window);
    drawTextBoxes(window);
    drawSubmit(window, hoverSubmit1);

    displayTextLogin(window, 360.f, 135.f);
    displayPasswordLogin(window, 360.f, 185.f);

    drawBack(window);
}

void drawThirdPage(sf::RenderWindow &window)
{
    sf::Texture texture;
    if (!texture.loadFromFile("image1.png"))
    {
        printf("Eroare la background\n");
    }

    sf::Sprite sprite;
    sprite.setTexture(texture);
    sprite.setTextureRect(sf::IntRect(0, 0, 1000, 600));

    window.draw(sprite);
    
    drawTextRegister(window);
    drawTextBoxes(window);
    drawSubmit(window, hoverSubmit2);

    displayTextLogin(window, 360.f, 135.f);
    displayPasswordLogin(window, 360.f, 185.f);

    drawBack(window);
}

void readInputLogin(sf::Event &input)
{
    if(currentPage == 2) {
        if(input.text.unicode == DELETE) {
            if(loginInput.length() > 0)
            {
                loginInput.pop_back();
            }
        }
            else 
        if(loginInput.length() < 11)
            loginInput += input.text.unicode;
    } else 
    if(currentPage == 3) {
        if(input.text.unicode == DELETE) {
            if(loginRegister.length() > 0)
            {
                loginRegister.pop_back();
            }
        }
            else 
        if(loginRegister.length() < 11)
            loginRegister += input.text.unicode;
    }
}

void readPasswordLogin(sf::Event &input)
{
    if(currentPage == 2) {
        if(input.text.unicode == DELETE) {
            if(passwordInput.length() > 0)
            {
                passwordInput.pop_back();
            }
        }
            else 
        if(passwordInput.length() < 20)
            passwordInput += input.text.unicode;
    } else 
    if(currentPage == 3) {
        if(input.text.unicode == DELETE) {
            if(passwordRegister.length() > 0)
            {
                passwordRegister.pop_back();
            }
        }
            else 
        if(passwordRegister.length() < 20)
            passwordRegister += input.text.unicode;
    }
}

bool mouseOverLogin(sf::RenderWindow &window)
{
    float x = sf::Mouse::getPosition(window).x;
    float y = sf::Mouse::getPosition(window).y;

    float loginX = 378.5f, loginY = 100.f;

    if(x >= loginX && x <= loginX + 243.f && y >= loginY && y <= loginY + 82.5f)
        return true;

    return false;
}

bool mouseOverRegister(sf::RenderWindow &window)
{
    float x = sf::Mouse::getPosition(window).x;
    float y = sf::Mouse::getPosition(window).y;

    float registerX = 342.5f, registerY = 200.f;

    if(x >= registerX && x <= registerX + 315.f && y >= registerY && y <= registerY + 82.5f)
        return true;

    return false;
}

bool mouseOverTextLogin(sf::RenderWindow & window)
{
    float x = sf::Mouse::getPosition(window).x;
    float y = sf::Mouse::getPosition(window).y;

    float box1X = 350.f, box1Y = 130.f;

    if(x >= box1X && x <= box1X + 300.f && y >= box1Y && y <= box1Y + 39.f)
        return true;

    return false;
}

bool mouseOverPasswordLogin(sf::RenderWindow & window)
{
    float x = sf::Mouse::getPosition(window).x;
    float y = sf::Mouse::getPosition(window).y;

    float box2X = 350.f, box2Y = 180.f;

    if(x >= box2X && x <= box2X + 300.f && y >= box2Y && y <= box2Y + 39.f)
        return true;

    return false;
}

bool mouseOverSubmit(sf::RenderWindow &window)
{
    float x = sf::Mouse::getPosition(window).x;
    float y = sf::Mouse::getPosition(window).y;

    float submitX = 425.f, submitY = 250.f;

    if(x >= submitX && x <= submitX + 150.f && y >= submitY && y <= submitY + 50.f)
        return true;

    return false;
}

void mouseMoved(sf::RenderWindow &window)
{
    if(currentPage == 1 && mouseOverLogin(window)) {
        hover1 = true;
    } else 
    if(currentPage == 1)
    {
        hover1 = false;
    }

    if(currentPage == 1 && mouseOverRegister(window)) {
        hover2 = true;
    } else 
    if(currentPage == 1)
    {
        hover2 = false;
    }

    if(currentPage == 2 && mouseOverSubmit(window))
    {
        hoverSubmit1 = true;
    } else 

    if(currentPage == 2)
    {
        hoverSubmit1 = false;
    } else 
    if(currentPage == 3 && mouseOverSubmit(window))
    {
        hoverSubmit2 = true;
    } else 
    if(currentPage == 3)
    {
        hoverSubmit2 = false;
    }
}

bool mouseOverBack(sf::RenderWindow &window)
{
    float x = sf::Mouse::getPosition(window).x;
    float y = sf::Mouse::getPosition(window).y;

    float backX = 25.f, backY = 25.f;

    if(x >= backX && x <= backX + 122.f && y >= backY && y <= backY + 50.f)
        return true;

    return false;
}

bool mouseOverAccident(sf::RenderWindow &window)
{
    float x = sf::Mouse::getPosition(window).x;
    float y = sf::Mouse::getPosition(window).y;

    float accidentX = 31.5f, accidentY = 100.f;

    if(x >= accidentX && x <= accidentX + 162.f && y >= accidentY && y <= accidentY + 52.f)
        return true;

    return false;
}

void sendProtocolAccident(int i, int j)
{
    string message = "accident:" + to_string(i) + "-" + to_string(j);
    
    write(sd, message.c_str(), message.length());
}

bool mouseOverDrive(sf::RenderWindow &window)
{
    float x = sf::Mouse::getPosition(window).x;
    float y = sf::Mouse::getPosition(window).y;

    float driveX = 31.5f, driveY = 175.f;

    if(x >= driveX && x <= driveX + 162.f && y >= driveY && y <= driveY + 50.f)
        return true;

    return false;
}

void printPath(vector<int> parent, int j)
{
    // Base Case : If j is source 
    if (parent[j] == -1) 
        return; 
  
    printPath(parent, parent[j]); 
    
    path[nr++] = j;
    //printf("%d ", j); 
    //fflush(stdout);
}

void requestSpeed(int p1, int p2)
{
    string speed_str = "-";
    int speedInt;

    escape = false;
    string protocol = "reqSpeed:" + to_string(p1) + ":" + to_string(p2);

    write(sd, protocol.c_str(), protocol.length());
    
}

void displaySpeed(sf::RenderWindow &window, int speed, string pretext, float x, float y)
{
    sf::Text textSpeed;
    textSpeed.setCharacterSize(20);
    textSpeed.setFillColor(sf::Color::Black);
    sf::Font arial;
    arial.loadFromFile("Arial.ttf");
    textSpeed.setFont(arial);

    if(speed == -1) {
        textSpeed.setString(pretext + " waiting for speed...");
    } else 
        textSpeed.setString(pretext + to_string(speed) + " km / h");

    textSpeed.setPosition({x, y});

    window.draw(textSpeed);
}

void showShortestPath(sf::RenderWindow &window, int x1, int x2)
{
    showPath = true;
    source = x1;
    x = locations[source].first + 30.f;
    y = locations[source].second;
    nr = 0;
    vector<bool> viz(locations.size(), false);
    priority_queue< pair<int, int> > q;
    vector<int> c(locations.size());
    vector<int> parent(locations.size());

    parent[x1] = -1;

    for(int i = 0; i < locations.size(); ++i) c[i] = 20000000;

    c[x1] = 0;
    q.push({0, x1});

    while(!q.empty()) {
        int a = q.top().second;
        q.pop();

        if(viz[a]) continue;
        viz[a] = true;

        for(int i = 0; i < locations.size(); ++i)
        if(matrix[a][i]) {
            int b = i;
            int w = cost[b][a];


            if(c[a] + w < c[b]) {
                parent[b] = a;
                c[b] = c[a] + w;
                q.push({-c[b], b});
            }
        }
    }
    
    
    printPath(parent, x2);
    for(int i = 0; i < nr; ++i) printf("%d ", path[i]), fflush(stdout);
    streetColor[x1][path[0]] = streetColor[path[0]][x1] = 2;

    for(int i = 0; i < nr - 1; ++i)
        streetColor[path[i]][path[i + 1]] = streetColor[path[i + 1]][path[i]] = 2;

    point1 = x1, point2 = path[0];

    if(point1 > point2) {
        requestSpeed(point2, point1);
    } else 
        requestSpeed(point1, point2);

    indexStreet = 0;
    //displaySpeed(window, speed, "Legal speed: ", 0.f, 0.f);
}

bool mouseOverInfo(sf::RenderWindow &window)
{
    float x = sf::Mouse::getPosition(window).x;
    float y = sf::Mouse::getPosition(window).y;

    float infoX = 31.5f, infoY = 250.f;

    if(x >= infoX && x <= infoX + 162.f && y >= infoY && y <= infoY + 50.f)
        return true;

    return false;
}

void redirectMessage(string message)
{
    //accident
    if(message.find("-") != -1) {

        string street = message;
        bool done = false;

        if(street[street.length() - 1] == 'G') street.pop_back(), done = true;
            int pos = street.find('-');

            int point1 = stoi(street.substr(0, pos)), point2 = stoi(street.substr(pos + 1, street.length() - pos));
            
            if(done) {
                streetColor[point1][point2] = streetColor[point2][point1] = 0;
                if(cost[point1][point2] > 1000) {
                    cost[point1][point2] -= 1000;
                    cost[point2][point1] = 1000;
                }
            }
            else {
                streetColor[point1][point2] = streetColor[point2][point1] = 1;
                cost[point1][point2] += 1000;
                cost[point2][point1] += 1000;
            }
    } else 
    //show info
    if(message.find(":") != -1) {

        string infos = message;
        int pos = infos.find(":");
        string size = infos.substr(0, pos);

        string info = infos.substr(pos + 1, stoi(size));

        int pos1 = info.find(":");

        fflush(stdout);
        cout << info << "\n";

        weather = info.substr(0, pos1);

        int pos2 = pos1 + 3;

        temp = info.substr(pos1 + 1, 2);

        sport = info.substr(pos2 + 1, info.length() - pos2);

        infoDisplay = true;
    } 
    //request speed
    else { 
        string speed_str = message;
        speed = stoi(speed_str);
    }
}

void stuffAfterConnection()
{
    int ret;

    //watch for reading
    fds[0].fd = sd;
    fds[0].events = POLLIN;

    ret = poll(fds, 1, 1000);

    if(ret == -1) {
        perror("poll");
        exit(1);
    }

    fflush(stdout);
    if(fds[0].revents & POLLIN) {
        printf("stdin is readable\n");
        char message[500];
        bzero(message, 500);

        int len = read(fds[0].fd, message, 200);

        if(!len) {
            fflush(stdout);
            cout << "Server disconnected!\n";
            stopProgram = true;
            pthread_exit(NULL);
        }

        string messg = message;

        redirectMessage(messg);

        fflush(stdout);
        printf("%s\n", message);
    }
}

void mousePressed(sf::RenderWindow &window)
{
    switch (currentPage)
    {
    case 1:
        if(mouseOverLogin(window)) {
            currentPage = 2;
            hoverSubmit1 = false;
            activeBox1 = false;
            activeBox2 = false;
        } else 
        if(mouseOverRegister(window)) {
            currentPage = 3;
            hoverSubmit2 = false;
            activeBox1 = false;
            activeBox2 = false;
        }
        break;
    case 2:
        if(mouseOverTextLogin(window)) {
            activeBox2 = false;
            activeBox1 = true;
        }
            else
        if(mouseOverPasswordLogin(window))
        {
            activeBox1 = false;
            activeBox2 = true;
        }
            else 
        if(mouseOverSubmit(window))
        {
            //while(blocked);

            //blocked = true;
            // Send login
            int len = loginInput.length();
            char message[200];
            bzero(message, 200);

            snprintf(message, 200, "login:%s", loginInput.c_str());
            write(sd, message, strlen(message));

            cout << loginInput << "\n";

            char answer[200];
            bzero(answer, 200);
            read(sd, answer, 200);

            string ans = answer;
            if(ans == "OK") { //send password

                write(sd, passwordInput.c_str(), passwordInput.length());

                bzero(answer, 200);
                read(sd, answer, 200);

                ans = answer;
                cout << ans << "\n";

                if(ans == "OK") {
                    fflush(stdout);
                    cout << "Conectat!\n";
                    connected = true;
                    currentPage = 4;
                    //stuffAfterConnection();
                } else {
                    fflush(stdout);
                    cout << "Parola incorecta\n";
                    loginInput = "";
                    passwordInput = "";
                }
            } else {
                fflush(stdout);
                cout << "Login incorect\n";
                loginInput = "";
                passwordInput = "";
            }

        }
            else 
        if(mouseOverBack(window)) {
            activeBox1 = false;
            activeBox2 = false;
            hover1 = false;
            hover2 = false;
            currentPage = 1;
        }
            else
        {
            activeBox1 = false;
            activeBox2 = false;
        }
            
        break;
    case 3:

        if(mouseOverTextLogin(window)) {
            activeBox1 = true;
            activeBox2 = false;
        }
            else 
        if(mouseOverPasswordLogin(window)) {
            activeBox1 = false;
            activeBox2 = true;
        }
            else 
        if(mouseOverSubmit(window)) {

            string message = "register:" + loginRegister + ":" + passwordRegister;

            write(sd, message.c_str(), message.length());

            currentPage = 1;
            activeBox1 = false;
            activeBox2 = false;
            hover1 = false;
            hover2 = false;
            // fflush(stdout);
            // cout << "|" << message << "|\n";

        } 
            else 
        if(mouseOverBack(window)) {
            activeBox1 = false;
            activeBox2 = false;
            hover1 = false;
            hover2 = false;
            currentPage = 1;
        }
        else {
            activeBox1 = false;
            activeBox2 = false;
        }
        break;
    case 4:
        if(waitingForStreet) {
            float x = sf::Mouse::getPosition(window).x;
            float y = sf::Mouse::getPosition(window).y;

            int n = streets.size();
            for(int i = 0; i < n; ++i) {
                float x1 = streets[i].first.first, y1 = streets[i].first.second;
                float x2 = streets[i].second.first, y2 = streets[i].second.second;
                
                if(x1 == x2) x2 += 15; else y2 += 15; 
                //cout << x << " " << y << "\n";

                if(x >= x1 && x <= x2 && y >= y1 && y <= y2) {
                    waitingForStreet = false;
                    sendProtocolAccident(streetsIndx[i].first, streetsIndx[i].second);
                    //streetColor[streetsIndx[i].first][streetsIndx[i].second] = 1;
                    printf("opa\n");
                    break;
                }
            }
        }
            else
        if(waitingForLoc1) {
            //cout << "ok\n";
            float x = sf::Mouse::getPosition(window).x;
            float y = sf::Mouse::getPosition(window).y;

            int n = locations.size();
            for(int i = 0; i < n; ++i)
            {
                float x1 = locations[i].first, y1 = locations[i].second;

                //cout << x1 << " " << y1 << " " << x << " " << y << "\n";

                if(x >= x1 + 30 && x <= x1 + 55 && y >= y1 && y <= y1 + 25) {
                    waitingForLoc1 = false;
                    loc1 = i;
                    waitingForLoc2 = true;
                    cout << "first\n";
                    break;
                }
            }
        }
            else 
        if(waitingForLoc2) {
            float x = sf::Mouse::getPosition(window).x;
            float y = sf::Mouse::getPosition(window).y;

            int n = locations.size();
            for(int i = 0; i < n; ++i)
            {
                float x1 = locations[i].first, y1 = locations[i].second;

                if(i != loc1 && x >= x1 + 30 && x <= x1 + 55 && y >= y1 && y <= y1 + 25) {
                    waitingForLoc2 = false;
                    loc2 = i;
                    printf("second\n");
                    fflush(stdout);
                    showShortestPath(window, loc1, loc2);
                    //cout << loc1 << " " << loc2 << "\n";
                    //sleep(0.5);
                    break;
                    //waitingForLoc2 = true;
                }
            }
        }
            else
        if(mouseOverAccident(window)) {
            waitingForStreet = true;
            waitingForLoc1 = false;
            waitingForLoc2 = false;
        }
            else 
        if(mouseOverDrive(window)) {
            cout << "go\n";
            waitingForStreet = false;
            waitingForLoc2 = false;
            waitingForLoc1 = true;
        }   else 
        if(mouseOverInfo(window) && !infoDisplay) {

            sport = "";
            weather = "";
            temp = "";

            write(sd, "info:", 4);

        }
        }
}

void showInfo(sf::RenderWindow &window)
{
    // cout << "da\n";
    sf::Texture infoBkTexture;
    if (!infoBkTexture.loadFromFile("infoBk.jpg"))
    {
        printf("Eroare la info\n");
    }
    infoBkTexture.setSmooth(true);
    sf::Sprite spriteInfoBk;
    spriteInfoBk.setTexture(infoBkTexture);
    spriteInfoBk.setPosition({19.5f, 350.f});

    spriteInfoBk.scale({0.25, 0.15});

    window.draw(spriteInfoBk);

    //vremea
    sf::Text textWeather;
    textWeather.setCharacterSize(20);
    textWeather.setFillColor(sf::Color::Black);
    sf::Font arial;
    arial.loadFromFile("Arial.ttf");
    textWeather.setFont(arial);
    textWeather.setString("Weather:  " + weather);

    //temperatura
    sf::Text textTemp;
    textTemp.setCharacterSize(20);
    textTemp.setFillColor(sf::Color::Black);
    //sf::Font arial;
    arial.loadFromFile("Arial.ttf");
    textTemp.setFont(arial);
    textTemp.setString("Temperature:  " + temp);

    //sport
    sf::Text textSport;
    textSport.setCharacterSize(20);
    textSport.setFillColor(sf::Color::Black);
    //sf::Font arial;
    arial.loadFromFile("Arial.ttf");
    textSport.setFont(arial);
    textSport.setString("Sport:  " + sport);

    //peco
    sf::Text textPeco;
    textPeco.setCharacterSize(20);
    textPeco.setFillColor(sf::Color::Black);
    //sf::Font arial;
    arial.loadFromFile("Arial.ttf");
    textPeco.setFont(arial);
    textPeco.setString("Peco:  1.5 euro/L");

    //positioning
    textWeather.setPosition({35, 400});
    textTemp.setPosition({35, 440});
    textSport.setPosition({35, 490});
    textPeco.setPosition({35, 535});

    //icons
    sf::Texture sunnyTexture;
    if (!sunnyTexture.loadFromFile("sunny.png"))
    {
        printf("Eroare la sunny\n");
    }
    sunnyTexture.setSmooth(true);
    sf::Sprite sprite_sunny;
    sprite_sunny.setTexture(sunnyTexture);
    sprite_sunny.setPosition({40.f, 355.f});

    sprite_sunny.scale({0.08, 0.08});

    window.draw(sprite_sunny);

    sf::Texture pecoTexture;
    if (!pecoTexture.loadFromFile("peco.png"))
    {
        printf("Eroare la peco\n");
    }
    pecoTexture.setSmooth(true);
    sf::Sprite sprite_peco;
    sprite_peco.setTexture(pecoTexture);
    sprite_peco.setPosition({86.9f, 355.f});

    sprite_peco.scale({0.08, 0.08});
    window.draw(sprite_peco);

    sf::Texture sportTexture;
    if (!sportTexture.loadFromFile("sport.png"))
    {
        printf("Eroare la sport\n");
    }
    sportTexture.setSmooth(true);
    sf::Sprite sprite_sport;
    sprite_sport.setTexture(sportTexture);
    sprite_sport.setPosition({140.f, 355.f});

    sprite_sport.scale({0.08, 0.08});
    window.draw(sprite_sport);

    window.draw(textWeather);
    window.draw(textTemp);
    window.draw(textSport);
    window.draw(textPeco);

}

void resetShortestPath()
{
    indexStreet = 0;
    showPath = false;

    streetColor[source][path[0]] = streetColor[path[0]][source] = 0;

    for(int i = 0; i < nr - 1; ++i)
        streetColor[path[i]][path[i + 1]] = streetColor[path[i + 1]][path[i]] = 0;

    nr = 0;
}

void drawCar(sf::RenderWindow &window)
{
    sf::Texture carTexture;
    if (!carTexture.loadFromFile("car.png"))
    {
        printf("Eroare la car\n");
    }
    carTexture.setSmooth(true);
    sf::Sprite sprite_car;
    sprite_car.setTexture(carTexture);
    sprite_car.scale({0.01, 0.01});
    sprite_car.setPosition({x, y});

    if(x == locations[point2].first + 30.f && y == locations[point2].second) { //move to next street
        ++indexStreet;
        point1 = point2;
        point2 = path[indexStreet];

        fflush(stdout);
        cout << point1 << " " << point2 << "\n";

        if(indexStreet == nr) {
            resetShortestPath();
            return;
        }

        if(point1 > point2)    
            requestSpeed(point2, point1);
        else
            requestSpeed(point1, point2);
    }

    if(locations[point1].first == locations[point2].first) { //up or down
        if(locations[point1].second < locations[point2].second) 
            y += 1.f; // down
        else 
            y -= 1.f; // up 
    } else { // left or right
        if(locations[point1].first < locations[point2].first)
            x += 1.f; // left
        else   
            x -= 1.f; // right
    }
    
    //x += 0.8f;

    window.draw(sprite_car);
}

void sayHello(sf::RenderWindow &window)
{
    sf::Text textName;
    textName.setCharacterSize(30);
    textName.setFillColor(sf::Color::Black);
    sf::Font comics;
    comics.loadFromFile("Comic_Sans_MS.ttf");
    textName.setFont(comics);
    textName.setString("Welcome " + loginInput);

    textName.setPosition({25.f, 8.f});
    window.draw(textName);

}

void drawMainPage(sf::RenderWindow &window)
{
    sf::Texture texture;
    if (!texture.loadFromFile("image1.png"))
    {
        printf("Eroare la background\n");
    }

    sf::Sprite sprite;
    sprite.setTexture(texture);
    sprite.setTextureRect(sf::IntRect(0, 0, 1000, 600));

    window.draw(sprite);

    sf::RectangleShape border(sf::Vector2f(771.f, 514.f));
    border.setFillColor(sf::Color::Black);
    border.setPosition({224.f, 43.25f});
    window.draw(border);

    sf::Texture texture1;
    if (!texture1.loadFromFile("bk.jpg"))
    {
        printf("Eroare la background\n");
    }
    sf::Sprite sprite1;
    sprite1.setTexture(texture1);
    //sprite1.setTextureRect(sf::IntRect(100, 100, 612, 441));
    sprite1.setPosition({225, 44.25});
    sprite1.scale({1.5, 1.5});
    
    window.draw(sprite1);

    int n = locations.size();

    for(int i = 0; i < n; ++i)
        for(int j = 0; j < n; ++j)
            if(matrix[i][j]) {

                for(float ii = -5; ii <= 5; ++ii)
                for(float jj = -5; jj <= 5; ++jj) {
                    sf::Vertex line[] =
                    {
                        sf::Vertex(sf::Vector2f(30 + locations[i].first + 10.f + ii, locations[i].second + 10.f + jj)),
                        sf::Vertex(sf::Vector2f(30 + locations[j].first + 10.f + ii, locations[j].second + 10.f + jj))
                    };
                    if(streetColor[i][j] == 0) {
                        line[0].color = sf::Color::Green;
                        line[1].color = sf::Color::Green;
                    } else 
                    if(streetColor[i][j] == 1) {
                        line[0].color = sf::Color::Red;
                        line[1].color = sf::Color::Red;
                    } else
                    if(streetColor[i][j] == 2) {
                        line[0].color = sf::Color::Blue;
                        line[1].color = sf::Color::Blue;
                    }
                    
                    window.draw(line, 2, sf::Lines);
                }
            }
    for(int i = 0; i < n; ++i) {
        sf::CircleShape circle(10.f);
        circle.setPosition({locations[i].first + 30, locations[i].second});
        circle.setFillColor(sf::Color::Black);
        window.draw(circle);
    }

    sf::Texture accidentTexture;
    if (!accidentTexture.loadFromFile("accident.png"))
    {
        printf("Eroare la accident\n");
    }
    accidentTexture.setSmooth(true);
    sf::Sprite spriteAccident;
    spriteAccident.setTexture(accidentTexture);
    spriteAccident.setPosition({31.5f, 100.f});

    sf::Texture driveTexture;
    if (!driveTexture.loadFromFile("drive.png"))
    {
        printf("Eroare la drive\n");
    }
    driveTexture.setSmooth(true);
    sf::Sprite spriteDrive;
    spriteDrive.setTexture(driveTexture);
    spriteDrive.setPosition({31.5f, 175.f});

    sf::Texture infoTexture;
    if (!infoTexture.loadFromFile("info.png"))
    {
        printf("Eroare la info\n");
    }
    infoTexture.setSmooth(true);
    sf::Sprite spriteInfo;
    spriteInfo.setTexture(infoTexture);
    spriteInfo.setPosition({31.5f, 250.f});
    //spriteAccident.scale({1.5, 1.5});

    if(infoDisplay) {
        showInfo(window);
    }

    if(showPath) {
        drawCar(window);
        displaySpeed(window, speed, "Legal speed: ", 300.f, 5.f);
        displaySpeed(window, realSpeed, "Current speed: ", 550.f, 5.f);
    }

    if(connected) {
        sayHello(window);
    }
    
    window.draw(spriteDrive);
    window.draw(spriteAccident);
    window.draw(spriteInfo);

}

void generateStreets()
{
    int n = locations.size();

    for(int i = 0; i < n; ++i)
        for(int j = 0; j < n; ++j)
            if(matrix[i][j]) {
                float x1 = locations[i].first + 30.f, y1 = locations[i].second;
                float x2 = locations[j].first + 30.f, y2 = locations[j].second;

                streets.push_back({{x1, y1}, {x2, y2}});
                streetsIndx.push_back({i, j});
                streetColor[i][j] = 0; //green
                cost[i][j] = max(abs(x1 - x2), abs(y1 - y2));
                //cout << cost[i][j] << " ";
            } else cost[i][j] = 0;
    for(int i = 0; i < n; ++i)
    for(int j = 0; j < n; ++j)
        if(matrix[i][j])
            matrix[j][i] = true, cost[j][i] = cost[i][j], streetColor[j][i] = streetColor[i][j];
}

void sendSpeed()
{
    string speedProtocol = "speed:";
    int speedRand = rand() % 141 + 10; // [10-150]

    realSpeed = speedRand;

    speedProtocol += to_string(speedRand);

    printf("%d\n", speedRand);
    write(sd, speedProtocol.c_str(), speedProtocol.length());
}

void readText(sf::RenderWindow &window, sf::Event event)
{
    if(currentPage == 2 && activeBox1)
        readInputLogin(event);
    else if(currentPage == 2 && activeBox2)
        readPasswordLogin(event);
    else if(currentPage == 3 && activeBox1)
        readInputLogin(event);
    else if(currentPage == 3 && activeBox2)
        readPasswordLogin(event);
}

int main (int argc, char *argv[])
{
    srand(time(0));
    connectToServer(argc, argv);

    thData * td1; 
    td1 = (struct thData*) malloc(sizeof(struct thData));
    td1->idThread=i++;
    pthread_create(&th[i], NULL, &waitUpdates, td1);

    sf::RenderWindow window(sf::VideoMode(1000, 600), "SFML works!");
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    generateStreets();

    auto start = high_resolution_clock::now();

    while (window.isOpen()) {

        if(stopProgram) break;

        auto stop = high_resolution_clock::now(); 

        auto duration = duration_cast<seconds>(stop - start); 
        
        if(duration.count() >= 60) {
            sendSpeed();
            start = high_resolution_clock::now();
        }
        //cout << duration.count() << "\n";
        // char protocol[100];
        // bzero(protocol, 100);
        // scanf("%s", protocol);
        // write(sd, protocol, 100);

        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                stopProgram = true;
                break;
            case sf::Event::MouseMoved:

                mouseMoved(window);

                break;
            case sf::Event::MouseButtonPressed:
                    
                mousePressed(window);
                
                break;
            case sf::Event::TextEntered:
                readText(window, event);

                break;
            default:
                break;
            }
        }
        
        

        window.clear();
        if(currentPage == 1)
        {
            drawFirstPage(window, hover1, hover2);
        }
            else
        if(currentPage == 2)
        {
            drawSecondPage(window);
        }
            else 
        if(currentPage == 3)
        {
            drawThirdPage(window);
        }
            else
        if(currentPage == 4)
        {
            drawMainPage(window);
        }
        
        
        //window.draw(sprite);
        //window.draw(shape);
        window.display();
    }

    close (sd);
    return 0;
}

static void *waitUpdates(void *arg) 
{
    struct thData tdL; 
    tdL= *((struct thData*)arg);

    pthread_detach(pthread_self());
    
    while(!connected) {
        if(stopProgram) break;
    }

    while(1) {
        if(stopProgram) break;
        stuffAfterConnection();
    }

    return(NULL);
}