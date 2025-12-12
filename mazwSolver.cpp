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
    bool operator<(const Node& o)  const { return x < o.x  (x == o.x && y < o.y); }
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
    this_thread::sleep_for(chrono::milliseconds(550));
}

/* ---------- Result ---------- */
struct Result {
    bool found;
    int  pathLen;
    int  expanded;
    int  pathCells;    //  P
    double timeSec;
};
/* ---------- BFS ---------- */
Result runBFS() {
    queue<Node> q;
    set<Node> vis;
    map<Node, Node> parent;
    q.push(startNode);
    vis.insert(startNode);

    int step = 0, pathLen = 0, pathCells = 0;   // عدّاد الخلايا المرسومة
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
    double t = duration<double>(en - st).count();

    clearScreen();
    if (found) {
        Node walk = goalNode;
        while (!(walk == startNode)) {
            if (maze[walk.x][walk.y] != START && maze[walk.x][walk.y] != GOAL) {
                maze[walk.x][walk.y] = PATH;
                ++pathCells;            // عدّ الخلايا اللي اتغيّرت
            }
            walk = parent[walk];
        }
        cout << "BFS solved maze:\n";
    } else {
        cout << "BFS failed – final explored state:\n";
    }
    for (auto& row : maze) { for (char c : row) cout << c; cout << '\n'; }

    // طباعة القيم الجديدة
    cout << "\nNodes expanded: " << step
         << "\nPath length   : " << pathLen
         << "\nPath cells    : " << pathCells
         << "\nTime          : " << t << " s\n\n";

    return {found, pathLen, step, pathCells, t};
}
/* ---------- DFS ---------- */
Result runDFS() {
    vector<Node> stack;
    set<Node> vis;
    map<Node, Node> parent;
    stack.push_back(startNode);
    vis.insert(startNode);

    int step = 0, pathLen = 0,pathCells = 0;
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
    double t = duration<double>(en - st).count();

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
    cout << "\nNodes expanded: " << step << "\nTime: " << t << " s\n\n";
    return {found, pathLen, step, pathCells, t};
}
/* ---- Manhattan Heuristic ---- */
int heuristic(const Node& a, const Node& b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

/* ---------- A*  ---------- */
Result runAstar() {

    using P = pair<int, Node>;  // f , node
    priority_queue<P, vector<P>, greater<P>> pq;

    map<Node, Node> parent;
    map<Node, int> g;

    g[startNode] = 0;
    pq.push({0, startNode});

    set<Node> vis;

    int step = 0, pathLen = 0,pathCells = 0;
    auto st = high_resolution_clock::now();
    bool found = false;

    while (!pq.empty()) {

        Node cur = pq.top().second;
        pq.pop();

        step++;
        printMaze(vis, cur, step);

        if (cur == goalNode) {
            found = true;
            break;
        }

        vis.insert(cur);

        int dx[] = {-1, 1, 0, 0};
        int dy[] = {0, 0, -1, 1};

        for (int k = 0; k < 4; k++) {
            int nx = cur.x + dx[k];
            int ny = cur.y + dy[k];
            Node nb(nx, ny);

            if (!passable(nx, ny)) continue;

            int newG = g[cur] + 1;

            if (!g.count(nb)  newG < g[nb]) {
                g[nb] = newG;
                int f = newG + heuristic(nb, goalNode);
                pq.push({f, nb});
                parent[nb] = cur;
            }
        }
    }

    auto en = high_resolution_clock::now();
    double t = duration<double>(en - st).count();

    clearScreen();

    if (found) {
        int pathLen = 0;
        Node cur = goalNode;

        while (!(cur == startNode)) {
            if (maze[cur.x][cur.y] != 'S' && maze[cur.x][cur.y] != 'G')
                maze[cur.x][cur.y] = 'P';
            cur = parent[cur];
            pathLen++;
        }

        cout << "A* solved maze:\n";
        for (auto& r : maze) { for (char c : r) cout << c; cout << '\n'; }



        cout << "\nNodes expanded: " << step << "\nTime: " << t << " s\n\n";
        cout << "\nPath length: " << pathLen;
      return {found, pathLen, step, pathCells, t};
    }

    else {
        cout << "A* failed.\n";
        return {false, 0, step, t};
    }
}
/* ---------- UCS ---------- */
Result runUCS() {
    using P = pair<int, Node>;  // cost, node
    priority_queue<P, vector<P>, greater<P>> pq;

    map<Node, int> g;
    map<Node, Node> parent;
    set<Node> vis;

    g[startNode] = 0;
    pq.push({0, startNode});

    int step = 0 ,pathLen = 0,pathCells = 0;
    auto st = high_resolution_clock::now();
    bool found = false;

    while (!pq.empty()) {
        Node cur = pq.top().second;
        int curCost = pq.top().first;
        pq.pop();
       step++;
        printMaze(vis, cur, step);

        if (cur == goalNode) {
            found = true;
            break;
        }

        if (vis.count(cur)) continue;
        vis.insert(cur);

        int dx[] = {-1, 1, 0, 0};
        int dy[] = {0, 0, -1, 1};

        for (int k = 0; k < 4; k++) {
            int nx = cur.x + dx[k];
            int ny = cur.y + dy[k];
            Node nb(nx, ny);

            if (!passable(nx, ny)) continue;

            int newG = g[cur] + 1;

            if (!g.count(nb) || newG < g[nb]) {
                g[nb] = newG;
                pq.push({newG, nb});
                parent[nb] = cur;
            }
        }
    }

    auto en = high_resolution_clock::now();
    double t = duration<double>(en - st).count();

    clearScreen();

    if (found) {
        int pathLen = 0;
        Node cur = goalNode;

        while (!(cur == startNode)) {
            if (maze[cur.x][cur.y] != 'S' && maze[cur.x][cur.y] != 'G')
                maze[cur.x][cur.y] = 'P';
            cur = parent[cur];
            pathLen++;
        }

        cout << "UCS solved maze:\n";
        for (auto& r : maze) { for (char c : r) cout << c; cout << '\n'; }

        cout << "\nNodes expanded: " << step << "\nTime: " << t << " s\n\n";
        cout << "\nPath length: " << pathLen;


        return {found, pathLen, step, pathCells, t};
    }

    cout << "UCS failed.\n";
    return {false, 0, step, t};
}
/* ---------- file savers ---------- */
void saveCSV(const string& algo, const Result& r, const string& label) {
    static bool headerWritten = false;
    ofstream f("results.csv", ios::app);
    if (!headerWritten) {
        f << "Algorithm,Maze,PathLen,Expanded,PathCells,TimeSec\n";
        headerWritten = true;
    }
    f << algo << ',' << label << ',' << r.pathLen << ',' << r.expanded << ',' << r.pathCells << ',' << r.timeSec << '\n';
}
void saveReport(const string& algo, const Result& r, const string& label) {
    ofstream f("report.txt", ios::app);
    f << "Algorithm: " << algo << "  (" << label << ")\n";
    f << "  Goal reached : " << (r.found ? "YES" : "NO") << '\n';
    f << "  Nodes expanded: " << r.expanded << '\n';
    f << "  Path length   : " << r.pathLen << '\n';
    f << "  Path cells    : " << r.pathCells << '\n';   // <===
    f << "  Time          : " << r.timeSec << " s\n\n";
}

int main() {
   
    vector<vector<vector<char>>> allMazes = loadAllMazes("mazes.txt");
    if (allMazes.empty()) {
        cout << "No mazes found in mazes.txt – exiting.\n";
        return 0;
    }

    cout << "=== Run search on every maze inside mazes.txt ===\n";
    cout << "Choose algorithm: (b) BFS  (d) DFS  (a) A*  (u) UCS : ";
    char choice;
    cin >> choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');   // flush rest of line

    string algo;
    switch (tolower(choice)) {
        case 'd': algo = "DFS";    break;
        case 'a': algo = "Astar";  break;
        case 'u': algo = "UCS";    break;
        default : algo = "BFS";    break;
    }

    /* ---------- 3.  solve each maze ---------- */
    for (size_t idx = 0; idx < allMazes.size(); ++idx) {
        maze = allMazes[idx];          // global maze updated

        /* find start and goal */
        for (int i = 0; i < (int)maze.size(); ++i)
            for (int j = 0; j < (int)maze[0].size(); ++j) {
                if (maze[i][j] == START) startNode = Node(i, j);
                if (maze[i][j] == GOAL)  goalNode  = Node(i, j);
            }

        cout << "\n---------- Maze #" << (idx + 1) << " (" << algo << ") ----------\n";

        Result r;
        if (algo == "DFS")        r = runDFS();
        else if (algo == "Astar") r = runAstar();
        else if (algo == "UCS")   r = runUCS();
        else                      r = runBFS();

        string label = "Maze-" + to_string(idx + 1);
        saveCSV(algo, r, label);
        saveReport(algo, r, label);
cout << "(Press Enter to continue to next maze...)";
        cin.ignore();


      saveReport(algo, r, label);
    }
    cout << "\nAll mazes processed.  Check results.csv & report.txt\n";
    return 0;
}
