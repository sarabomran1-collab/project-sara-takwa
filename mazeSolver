#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include <map>
#include <chrono>
#include <thread>
#include <fstream>

using namespace std;
using namespace std::chrono;

/* ---------- basic types ---------- */
struct Node {
    int x, y;
    Node(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Node& o) const { return x == o.x && y == o.y; }
    bool operator<(const Node& o)  const { return x < o.x || (x == o.x && y < o.y); }
};

/* ---------- globals ---------- */
vector<vector<char>> maze;
Node startNode, goalNode;
const char WALL  = '%';
const char FREE  = '.';
const char START = 'S';
const char GOAL  = 'G';
const char VIS   = '*';
const char CUR   = '+';
const char PATH  = 'P';

/* ---------- file loader (all mazes) ---------- */
vector<vector<vector<char>>> loadAllMazes(const string& fileName) {
    ifstream in(fileName);
    if (!in) { cout << "Error: cannot open " << fileName << '\n'; exit(1); }
    vector<vector<vector<char>>> list;
    vector<vector<char>> one;
    string line;
    while (getline(in, line)) {
        if (!line.empty()) one.emplace_back(line.begin(), line.end());
        else if (!one.empty()) { list.push_back(one); one.clear(); }
    }
    if (!one.empty()) list.push_back(one);
    return list;
}

/* ---------- utility ---------- */
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

/* ---------- print maze ---------- */
void printMaze(const set<Node>& vis, Node current, int step) {
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
    this_thread::sleep_for(chrono::milliseconds(100));
}

/* ---------- Result ---------- */
struct Result {
    bool found;
    int pathLen;
    int expanded;
    double timeMS;
};

/* ---------- BFS ---------- */
Result runBFS() {
    queue<Node> q;
    set<Node> vis;
    map<Node, Node> parent;
    q.push(startNode);
    vis.insert(startNode);

    int step = 0;
    auto st = high_resolution_clock::now();
    bool found = false;

    while (!q.empty()) {
        Node cur = q.front(); q.pop();
        step++;
        printMaze(vis, cur, step);
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
    double t = duration<double, milli>(en - st).count();

    clearScreen();
    if (found) {
        Node walk = goalNode;
        while (!(walk == startNode)) {
            if (maze[walk.x][walk.y] != START && maze[walk.x][walk.y] != GOAL)
                maze[walk.x][walk.y] = PATH;
            walk = parent[walk];
        }
        cout << "BFS solved maze:\n";
    } else {
        cout << "BFS failed – final explored state:\n";
    }
    for (auto& row : maze) { for (char c : row) cout << c; cout << '\n'; }
    cout << "\nNodes expanded: " << step << "\nTime: " << t << " ms\n\n";
    return {found, 0, step, t};
}

/* ---------- DFS ---------- */
Result runDFS() {
    vector<Node> stack;
    set<Node> vis;
    map<Node, Node> parent;
    stack.push_back(startNode);
    vis.insert(startNode);

    int step = 0;
    auto st = high_resolution_clock::now();
    bool found = false;

    while (!stack.empty()) {
        Node cur = stack.back(); stack.pop_back();
        step++;
        printMaze(vis, cur, step);
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
    double t = duration<double, milli>(en - st).count();

    clearScreen();
    if (found) {
        Node walk = goalNode;
        while (!(walk == startNode)) {
            if (maze[walk.x][walk.y] != START && maze[walk.x][walk.y] != GOAL)
                maze[walk.x][walk.y] = PATH;
            walk = parent[walk];
        }
        cout << "DFS solved maze:\n";
    } else {
        cout << "DFS failed – final explored state:\n";
    }
    for (auto& row : maze) { for (char c : row) cout << c; cout << '\n'; }
    cout << "\nNodes expanded: " << step << "\nTime: " << t << " ms\n\n";
    return {found, 0, step, t};
}

/* ---------- file savers ---------- */
void saveCSV(const string& algo, const Result& r, const string& label) {
    ofstream f("results.csv", ios::app);
    if (f.tellp() == 0) f << "Algorithm,Maze,PathLen,Expanded,TimeMS\n";
    f << algo << "," << label << "," << r.pathLen << "," << r.expanded << "," << r.timeMS << "\n";
}
void saveReport(const string& algo, const Result& r, const string& label) {
    ofstream f("report.txt", ios::app);
    f << "Algorithm: " << algo << "  (" << label << ")\n";
    f << "  Goal reached: " << (r.found ? "YES" : "NO") << "\n";
    f << "  Nodes expanded: " << r.expanded << "\n";
    f << "  Time (ms): " << r.timeMS << "\n\n";
}

/* ---------- main ---------- */
int main() {
    cout << "=== BFS or DFS on every maze inside mazes.txt ===\n";
    cout << "Choose algorithm: (b) BFS  or  (d) DFS ? ";
    char choice; cin >> choice;
    string algo = (choice=='d'||choice=='D') ? "DFS" : "BFS";

    for (size_t idx = 0; idx < allMazes.size(); ++idx) {
        maze = allMazes[idx];
        for (int i = 0; i < (int)maze.size(); ++i)
            for (int j = 0; j < (int)maze[0].size(); ++j) {
                if (maze[i][j] == START) startNode = Node(i, j);
                if (maze[i][j] == GOAL)  goalNode  = Node(i, j);
            }
        cout << "\n---------- Maze #" << (idx+1) << " (" << algo << ") ----------\n";
        Result r = (algo=="DFS") ? runDFS() : runBFS();
        string label = "Maze-" + to_string(idx+1);
        saveCSV(algo, r, label);
        saveReport(algo, r, label);
        cout << "(Press Enter to continue to next maze...)";
        cin.ignore(); cin.ignore();
    }
    cout << "\nAll mazes processed. Check results.csv & report.txt\n";
    return 0;
}
