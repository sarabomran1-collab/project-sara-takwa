
#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include <map>
#include <chrono>
#include <thread>
#include <fstream>
#include <cctype>
#include <limits>

using namespace std;
using namespace std::chrono;

/* ---------- basics ---------- */
struct Node {
    int x, y;
    Node(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Node& o) const { return x == o.x && y == o.y; }
    bool operator<(const Node& o)  const { return x < o.x  (x == o.x && y < o.y); }
};

/* ---------- globals ---------- */
vector<vector<char>> maze;
Node startNode, goalNode;
const char WALL  = '%', FREE  = '.', START = 'S', GOAL  = 'G';
const char VIS   = '*', CUR   = '+', PATH  = 'P';
bool animate = false;     

/* ---------- utils ---------- */
bool inBounds(int x, int y) {
    return x >= 0 && y >= 0 && x < (int)maze.size() && y < (int)maze[0].size();
}
bool passable(int x, int y) {
    return inBounds(x, y) && maze[x][y] != WALL;
}
void clearScreen() {
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
}

/* ---------- print maze  ---------- */
void printMaze(const set<Node>& vis, Node current, int step) {
    if (!animate) return;
    clearScreen();
    cout << "Step " << step << ":\n";
    for (int i = 0; i < (int)maze.size(); ++i) {
        for (int j = 0; j < (int)maze[0].size(); ++j) {
            Node here(i, j);
            if (here == current)      cout << CUR;
            else if (vis.count(here)) cout << VIS;
            else                      cout << maze[i][j];
        }
        cout << '\n';
    }
    cout.flush();
    this_thread::sleep_for(chrono::milliseconds(200));
}

/* ---------- file loader (keyed by first line) ---------- */
map<string, vector<vector<char>>> loadAllMazes(const string& fileName) {
    ifstream in(fileName);
    if (!in) { cout << "Error: cannot open " << fileName << '\n'; exit(1); }
    map<string, vector<vector<char>>> outMap;
    string firstLine;
    while (getline(in, firstLine)) {
        vector<vector<char>> one;
        string line;
        while (getline(in, line)) {
            if (line.empty()) break;
            one.emplace_back(line.begin(), line.end());
        }
        if (!one.empty()) outMap[firstLine] = one;
    }
    return outMap;
}

/* ---------- Result ---------- */
struct Result {
    bool found;
    int  expanded;
    int  pathCells;
    double timeMs;
};

/* ---------- BFS ---------- */
Result runBFS() {
    queue<Node> q;
    set<Node> vis;
    map<Node, Node> parent;

    q.push(startNode);
    vis.insert(startNode);

    int expanded = 0, pathCells = 0;
    auto st = high_resolution_clock::now();
    bool found = false;

    while (!q.empty()) {
        Node cur = q.front(); q.pop();
        ++expanded;
        printMaze(vis, cur, expanded);
        if (cur == goalNode) { found = true; break; }

        int dx[] = {-1, 1, 0, 0}, dy[] = {0, 0, -1, 1};
        for (int d = 0; d < 4; ++d) {
            int nx = cur.x + dx[d], ny = cur.y + dy[d];
            Node nb(nx, ny);
            if (passable(nx, ny) && !vis.count(nb)) {
                vis.insert(nb);
                parent[nb] = cur;
                q.push(nb);
            }
        }
    }

    auto en = high_resolution_clock::now();
    double tMs = duration<double, milli>(en - st).count();

    if (found) {
        Node walk = goalNode;
        while (!(walk == startNode)) {
            if (maze[walk.x][walk.y] != START && maze[walk.x][walk.y] != GOAL) {
                maze[walk.x][walk.y] = PATH;
                ++pathCells;
            }
            walk = parent[walk];
        }
    }

    cout << (found ? "BFS solved maze:\n" : "BFS failed – final state:\n");
    for (auto& row : maze) { for (char c : row) cout << c; cout << '\n'; }
    cout << "Nodes expanded: " << expanded
         << "\nPath cells    : " << pathCells
         << "\nTime          : " << tMs / 1000.0 << " s\n\n";

    return {found, expanded, pathCells, tMs};
}

/* ---------- DFS ---------- */
Result runDFS() {
    vector<Node> stack;
    set<Node> vis;
    map<Nod

e, Node> parent;

    stack.push_back(startNode);
    vis.insert(startNode);

    int expanded = 0, pathCells = 0;
    auto st = high_resolution_clock::now();
    bool found = false;

    while (!stack.empty()) {
        Node cur = stack.back(); stack.pop_back();
        ++expanded;
        printMaze(vis, cur, expanded);
        if (cur == goalNode) { found = true; break; }

        int dx[] = {-1, 1, 0, 0}, dy[] = {0, 0, -1, 1};
        for (int d = 0; d < 4; ++d) {
            int nx = cur.x + dx[d], ny = cur.y + dy[d];
            Node nb(nx, ny);
            if (passable(nx, ny) && !vis.count(nb)) {
                vis.insert(nb);
                parent[nb] = cur;
                stack.push_back(nb);
            }
        }
    }

    auto en = high_resolution_clock::now();
    double tMs = duration<double, milli>(en - st).count();

    if (found) {
        Node walk = goalNode;
        while (!(walk == startNode)) {
            if (maze[walk.x][walk.y] != START && maze[walk.x][walk.y] != GOAL) {
                maze[walk.x][walk.y] = PATH;
                ++pathCells;
            }
            walk = parent[walk];
        }
    }

    cout << (found ? "DFS solved maze:\n" : "DFS failed – final state:\n");
    for (auto& row : maze) { for (char c : row) cout << c; cout << '\n'; }
    cout << "Nodes expanded: " << expanded
         << "\nPath cells    : " << pathCells
         << "\nTime          : " << tMs / 1000.0 << " s\n\n";

    return {found, expanded, pathCells, tMs};
}

/* ---------- heuristic ---------- */
int heuristic(const Node& a, const Node& b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

/* ---------- A*  ---------- */
Result runAstar() {
    using P = pair<int, Node>;          // {f, node}
    priority_queue<P, vector<P>, greater<P>> pq;

    map<Node, int> gScore;              
    map<Node, Node> parent;

    auto fScore = [&](const Node& n) {
        return gScore[n] + heuristic(n, goalNode);
    };

    gScore[startNode] = 0;
    pq.push({fScore(startNode), startNode});

    int expanded = 0, pathCells = 0;
    auto st = high_resolution_clock::now();
    bool found = false;

    while (!pq.empty()) {
        Node cur = pq.top().second;
        pq.pop();
        ++expanded;
        printMaze(set<Node>(), cur, expanded);   

        if (cur == goalNode) { found = true; break; }

        int dx[] = {-1, 1, 0, 0}, dy[] = {0, 0, -1, 1};
        for (int d = 0; d < 4; ++d) {
            int nx = cur.x + dx[d], ny = cur.y + dy[d];
            Node nb(nx, ny);
            if (!passable(nx, ny)) continue;

            int tentativeG = gScore[cur] + 1;
            if (!gScore.count(nb)  tentativeG < gScore[nb]) {
                gScore[nb] = tentativeG;
                parent[nb] = cur;
                pq.push({fScore(nb), nb});
            }
        }
    }

    auto en = high_resolution_clock::now();
    double tMs = duration<double, milli>(en - st).count();

    if (found) {
        Node walk = goalNode;
        while (!(walk == startNode)) {
            if (maze[walk.x][walk.y] != START && maze[walk.x][walk.y] != GOAL) {
                maze[walk.x][walk.y] = PATH;
                ++pathCells;
            }
            walk = parent[walk];
        }
    }

    cout << (found ? "A* solved maze:\n" : "A* failed – final state:\n");
    for (auto& row : maze) { for (char c : row) cout << c; cout << '\n'; }
    cout << "Nodes expanded: " << expanded
         << "\nPath cells    : " << pathCells
         << "\nTime          : " << tMs / 1000.0 << " s\n\n";

    return {found, expanded, pathCells, tMs};
}
/* ---------- UCS ---------- */
Result runUCS() {
    using P = pair<int, Node>;
    priority_queue<P, vector<P>, greater<P>> pq;
    map<Node, int> g;
    map<Node, Node> parent;
    set<Node> vis;

    g[startNode] = 0;
    pq.push({0, startNode});

    int expanded = 0, pathCells = 0;
    auto st = high_resolution_clock::now();
    bool found = false;

while (!pq.empty()) {
        Node cur = pq.top().second;
        pq.pop();
        ++expanded;
        printMaze(vis, cur, expanded);
        if (cur == goalNode) { found = true; break; }
        if (vis.count(cur)) continue;
        vis.insert(cur);

        int dx[] = {-1, 1, 0, 0}, dy[] = {0, 0, -1, 1};
        for (int d = 0; d < 4; ++d) {
            int nx = cur.x + dx[d], ny = cur.y + dy[d];
            Node nb(nx, ny);
            if (!passable(nx, ny)) continue;
            int newG = g[cur] + 1;
            if (!g.count(nb)  newG < g[nb]) {
                g[nb] = newG;
                pq.push({newG, nb});
                parent[nb] = cur;
            }
        }
    }

    auto en = high_resolution_clock::now();
    double tMs = duration<double, milli>(en - st).count();

    if (found) {
        Node walk = goalNode;
        while (!(walk == startNode)) {
            if (maze[walk.x][walk.y] != START && maze[walk.x][walk.y] != GOAL) {
                maze[walk.x][walk.y] = PATH;
                ++pathCells;
            }
            walk = parent[walk];
        }
    }

    cout << (found ? "UCS solved maze:\n" : "UCS failed – final state:\n");
    for (auto& row : maze) { for (char c : row) cout << c; cout << '\n'; }
    cout << "Nodes expanded: " << expanded
         << "\nPath cells    : " << pathCells
         << "\nTime          : " << tMs / 1000.0 << " s\n\n";

    return {found, expanded, pathCells, tMs};
}

/* ---------- GBFS ---------- */
Result runGBFS() {
    using P = pair<int, Node>;
    priority_queue<P, vector<P>, greater<P>> pq;
    map<Node, Node> parent;
    set<Node> vis;

    pq.push({heuristic(startNode, goalNode), startNode});

    int expanded = 0, pathCells = 0;
    auto st = high_resolution_clock::now();
    bool found = false;

    while (!pq.empty()) {
        Node cur = pq.top().second;
        pq.pop();
        ++expanded;
        printMaze(vis, cur, expanded);
        if (cur == goalNode) { found = true; break; }
        if (vis.count(cur)) continue;
        vis.insert(cur);

        int dx[] = {-1, 1, 0, 0}, dy[] = {0, 0, -1, 1};
        for (int d = 0; d < 4; ++d) {
            int nx = cur.x + dx[d], ny = cur.y + dy[d];
            Node nb(nx, ny);
            if (!passable(nx, ny)  vis.count(nb)) continue;
            parent[nb] = cur;
            pq.push({heuristic(nb, goalNode), nb});
        }
    }

    auto en = high_resolution_clock::now();
    double tMs = duration<double, milli>(en - st).count();

    if (found) {
        Node walk = goalNode;
        while (!(walk == startNode)) {
            if (maze[walk.x][walk.y] != START && maze[walk.x][walk.y] != GOAL) {
                maze[walk.x][walk.y] = PATH;
                ++pathCells;
            }
            walk = parent[walk];
        }
    }

    cout << (found ? "GBFS solved maze:\n" : "GBFS failed – final state:\n");
    for (auto& row : maze) { for (char c : row) cout << c; cout << '\n'; }
    cout << "Nodes expanded: " << expanded
         << "\nPath cells    : " << pathCells
         << "\nTime          : " << tMs / 1000.0 << " s\n\n";

    return {found, expanded, pathCells, tMs};
}

/* ---------- file savers ---------- */
void saveCSV(const string& algo, const Result& r, const string& label) {
    static bool headerWritten = false;
    ofstream f("results.csv", ios::app);
    if (!headerWritten) {
        f << "Algorithm,Maze,Expanded,PathCells,TimeSec\n";
        headerWritten = true;
    }
    f << algo << ',' << label << ',' << r.expanded << ',' << r.pathCells << ',' << r.timeMs / 1000.0 << '\n';
}

void saveReport(const string& algo, const Result& r, const string& label) {
    ofstream f("report.txt", ios::app);
    f << "Algorithm: " << algo << "  (" << label << ")\n"
      << "  Goal reached : " << (r.found ? "YES" : "NO") << '\n'
      << "  Nodes expanded: " << r.expanded << '\n'
      << "  Path cells    : " << r.pathCells << '\n'
      << "  Time          : " << r.timeMs / 1000.0 << " s\n\n";
}


/* ---------- main ---------- */
int main() {
    map<string, vector<vector<char>>> mazesMap = loadAllMazes("mazes.txt");
    if (mazesMap.empty()) {
        cout << "No mazes found in mazes.txt – exiting.\n";
        return 0;
    }

    cout << "=== Run search on every maze inside mazes.txt ===\n";
    cout << "Choose algorithm: (b) BFS  (d) DFS  (a) A*  (u) UCS  (g) GBFS : ";
    char algoChoice;
    cin >> algoChoice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string algo;
    switch (tolower(algoChoice)) {
        case 'd': algo = "DFS";    break;
        case 'a': algo = "Astar";  break;
        case 'u': algo = "UCS";    break;
        case 'g': algo = "GBFS";   break;
        default:  algo = "BFS";    break;
    }

    cout << "Choose difficulty: (e) easy  (m) medium  (h) hard  (a) all : ";
    char lvl;
    cin >> lvl;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string wanted;
    switch (tolower(lvl)) {
        case 'e': wanted = "easy_10x10";   break;
        case 'm': wanted = "medium_10x10"; break;
        case 'h': wanted = "hard_10x10";   break;
        default:  wanted = "";             // all
    }
    animate = (!wanted.empty());   

    for (auto& [key, oneMaze] : mazesMap) {
        if (!wanted.empty() && key != wanted) continue;
        maze = oneMaze;

        for (int i = 0; i < (int)maze.size(); ++i)
            for (int j = 0; j < (int)maze[0].size(); ++j) {
                if (maze[i][j] == START) startNode = Node(i, j);
                if (maze[i][j] == GOAL)  goalNode  = Node(i, j);
            }

        cout << "\n---------- " << key << " (" << algo << ") ----------\n";

        Result r;
        if      (algo == "DFS")  r = runDFS();
        else if (algo == "Astar") r = runAstar();
        else if (algo == "UCS")  r = runUCS();
        else if (algo == "GBFS") r = runGBFS();
        else                     r = runBFS();

        saveCSV(algo, r, key);
        saveReport(algo, r, key);

        cout << "(Press Enter to continue...)";
        cin.get();
    }

    cout << "\nAll mazes processed.  Check results.csv & report.txt\n";
    return 0;
}
