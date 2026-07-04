#include <iostream>
#include <vector>
#include <queue>
#include <chrono>

using namespace std;
using namespace std::chrono;

class Sudoku {
private:
    int grid[9][9];
    int original[9][9];

    // التحقق من الصف
    bool usedInRow(int row, int val) const {
        for (int c = 0; c < 9; c++)
            if (grid[row][c] == val) return true;
        return false;
    }

    // التحقق من العمود
    bool usedInCol(int col, int val) const {
        for (int r = 0; r < 9; r++)
            if (grid[r][col] == val) return true;
        return false;
    }

    // التحقق من مربع 3x3
    bool usedInBox(int row, int col, int val) const {
        int sr = (row / 3) * 3, sc = (col / 3) * 3;
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++)
                if (grid[sr + r][sc + c] == val) return true;
        return false;
    }

public:
    int stepsDFS = 0, backtracksDFS = 0;
    int stepsFC = 0, backtracksFC = 0;
    int stepsAC3 = 0, backtracksAC3 = 0;

    Sudoku() {
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                grid[r][c] = original[r][c] = 0;
    }

    // حفظ اللوحة الأصلية
    void copyOriginal() {
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                original[r][c] = grid[r][c];
    }

    // استرجاع اللوحة الأصلية
    void restoreOriginal() {
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                grid[r][c] = original[r][c];
    }

    // إدخال لوحة السودوكو
    void readFromUser() {
        cout << "أدخل لوحة السودوكو (0 للخانة الفارغة):\n";
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                cin >> grid[r][c];
        copyOriginal();
    }

    // طباعة اللوحة
    void printBoard() const {
        for (int r = 0; r < 9; r++) {
            if (r % 3 == 0) cout << "+-------+-------+-------+\n";
            for (int c = 0; c < 9; c++) {
                if (c % 3 == 0) cout << "| ";
                if (grid[r][c] == 0) cout << ". ";
                else cout << grid[r][c] << " ";
            }
            cout << "|\n";
        }
        cout << "+-------+-------+-------+\n";
    }

    // هل الرقم مناسب في الخانة؟
    bool isSafe(int row, int col, int val) const {
        return !usedInRow(row, val) && !usedInCol(col, val) && !usedInBox(row, col, val);
    }

    // أول خانة فارغة
    bool findEmpty(int& row, int& col) const {
        for (row = 0; row < 9; row++)
            for (col = 0; col < 9; col++)
                if (grid[row][col] == 0) return true;
        return false;
    }

    // القيم الممكنة لخانة
    vector<int> getPossibleValues(int row, int col) const {
        vector<int> vals;
        if (grid[row][col] != 0) return vals;
        for (int v = 1; v <= 9; v++)
            if (isSafe(row, col, v)) vals.push_back(v);
        return vals;
    }

    // DFS = Backtracking
    bool solveDFS() {
        int row, col;
        if (!findEmpty(row, col)) return true;

        for (int num = 1; num <= 9; num++) {
            stepsDFS++;
            if (isSafe(row, col, num)) {
                grid[row][col] = num;
                if (solveDFS()) return true;
                grid[row][col] = 0;
                backtracksDFS++;
            }
        }
        return false;
    }

    // فحص إذا بقيت أي خانة بدون قيم ممكنة
    bool forwardCheckBoard() const {
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                if (grid[r][c] == 0 && getPossibleValues(r, c).empty())
                    return false;
        return true;
    }

    // DFS مع Forward Checking
    bool solveForwardChecking() {
        int row, col;
        if (!findEmpty(row, col)) return true;

        vector<int> vals = getPossibleValues(row, col);
        for (int num : vals) {
            stepsFC++;
            grid[row][col] = num;
            if (forwardCheckBoard() && solveForwardChecking()) return true;
            grid[row][col] = 0;
            backtracksFC++;
        }
        return false;
    }

    // تهيئة المجالات
    vector<vector<vector<int>>> initDomains() const {
        vector<vector<vector<int>>> domains(9, vector<vector<int>>(9));
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                domains[r][c] = (grid[r][c] != 0) ? vector<int>{grid[r][c]} : getPossibleValues(r, c);
        return domains;
    }

    // تقليص المجال في AC-3
    bool revise(vector<vector<vector<int>>>& domains, int xi_r, int xi_c, int xj_r, int xj_c) const {
        bool revised = false;
        vector<int> newDomain;

        for (int x : domains[xi_r][xi_c]) {
            bool supported = false;
            for (int y : domains[xj_r][xj_c]) {
                if (x != y) {
                    supported = true;
                    break;
                }
            }
            if (supported) newDomain.push_back(x);
            else revised = true;
        }

        domains[xi_r][xi_c] = newDomain;
        return revised;
    }

    // AC-3
    bool ac3(vector<vector<vector<int>>>& domains) const {
        queue<pair<pair<int,int>, pair<int,int>>> q;

        for (int r1 = 0; r1 < 9; r1++)
            for (int c1 = 0; c1 < 9; c1++)
                for (int r2 = 0; r2 < 9; r2++)
                    for (int c2 = 0; c2 < 9; c2++)
                        if (!(r1 == r2 && c1 == c2) &&
                            (r1 == r2 || c1 == c2 || (r1 / 3 == r2 / 3 && c1 / 3 == c2 / 3)))
                            q.push({{r1, c1}, {r2, c2}});

        while (!q.empty()) {
            auto arc = q.front();
            q.pop();

            int xi_r = arc.first.first;
            int xi_c = arc.first.second;
            int xj_r = arc.second.first;
            int xj_c = arc.second.second;

            if (revise(domains, xi_r, xi_c, xj_r, xj_c)) {
                if (domains[xi_r][xi_c].empty()) return false;
            }
        }
        return true;
    }

    // AC-3 + DFS
    bool solveAC3Recursive(vector<vector<vector<int>>> domains) {
        int row = -1, col = -1, minSize = 10;

        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                if (grid[r][c] == 0 && (int)domains[r][c].size() < minSize) {
                    minSize = domains[r][c].size();
                    row = r;
                    col = c;
                }

        if (row == -1) return true;

        for (int num : domains[row][col]) {
            stepsAC3++;
            if (isSafe(row, col, num)) {
                int backup[9][9];
                for (int r = 0; r < 9; r++)
                    for (int c = 0; c < 9; c++)
                        backup[r][c] = grid[r][c];

                grid[row][col] = num;
                auto newDomains = domains;
                newDomains[row][col] = {num};

                if (ac3(newDomains) && solveAC3Recursive(newDomains)) return true;

                for (int r = 0; r < 9; r++)
                    for (int c = 0; c < 9; c++)
                        grid[r][c] = backup[r][c];

                backtracksAC3++;
            }
        }
        return false;
    }

    bool solveAC3() {
        auto domains = initDomains();
        if (!ac3(domains)) return false;
        return solveAC3Recursive(domains);
    }

    // قياس الزمن مع أي خوارزمية
    template<typename Func>
    long long measureTime(Func func) {
        auto start = high_resolution_clock::now();
        bool result = func();
        auto end = high_resolution_clock::now();
        long long ms = duration_cast<milliseconds>(end - start).count();
        cout << (result ? "تم الحل بنجاح.\n" : "لا يوجد حل.\n");
        return ms;
    }

    // مقارنة الخوارزميات
    void compareSolvers() {
        cout << "\nاللوحة الأصلية:\n";
        printBoard();

        restoreOriginal();
        stepsDFS = backtracksDFS = 0;
        cout << "\nDFS:\n";
        long long t1 = measureTime([&]() { return solveDFS(); });
        printBoard();
        cout << "Steps = " << stepsDFS << "\n";
        cout << "Backtracks = " << backtracksDFS << "\n";
        cout << "Time = " << t1 << " ms\n";

        restoreOriginal();
        stepsFC = backtracksFC = 0;
        cout << "\nForward Checking:\n";
        long long t2 = measureTime([&]() { return solveForwardChecking(); });
        printBoard();
        cout << "Steps = " << stepsFC << "\n";
        cout << "Backtracks = " << backtracksFC << "\n";
        cout << "Time = " << t2 << " ms\n";

        restoreOriginal();
        stepsAC3 = backtracksAC3 = 0;
        cout << "\nAC-3:\n";
        long long t3 = measureTime([&]() { return solveAC3(); });
        printBoard();
        cout << "Steps = " << stepsAC3 << "\n";
        cout << "Backtracks = " << backtracksAC3 << "\n";
        cout << "Time = " << t3 << " ms\n";

        restoreOriginal();
    }
};

int main() {
    Sudoku s;
    int choice;

    while (true) {
        cout << "\n1) إدخال لوحة سودوكو\n";
        cout << "2) حل DFS\n";
        cout << "3) مسح اللوحة\n";
        cout << "4) خروج\n";
        cout << "5) حل Forward Checking\n";
        cout << "6) حل AC-3\n";
        cout << "7) مقارنة الخوارزميات\n";
        cout << "اختر: ";
        cin >> choice;

        if (choice == 1) {
            s.readFromUser();
        }
        else if (choice == 2) {
            s.stepsDFS = s.backtracksDFS = 0;
            long long t = s.measureTime([&]() { return s.solveDFS(); });
            s.printBoard();
            cout << "Steps = " << s.stepsDFS << "\n";
            cout << "Backtracks = " << s.backtracksDFS << "\n";
            cout << "Time = " << t << " ms\n";
        }
        else if (choice == 3) {
            s = Sudoku();
        }
        else if (choice == 4) {
            break;
        }
        else if (choice == 5) {
            s.restoreOriginal();
            s.stepsFC = s.backtracksFC = 0;
            long long t = s.measureTime([&]() { return s.solveForwardChecking(); });
            s.printBoard();
            cout << "Steps = " << s.stepsFC << "\n";
            cout << "Backtracks = " << s.backtracksFC << "\n";
            cout << "Time = " << t << " ms\n";
        }
        else if (choice == 6) {
            s.restoreOriginal();
            s.stepsAC3 = s.backtracksAC3 = 0;
            long long t = s.measureTime([&]() { return s.solveAC3(); });
            s.printBoard();
            cout << "Steps = " << s.stepsAC3 << "\n";
            cout << "Backtracks = " << s.backtracksAC3 << "\n";
            cout << "Time = " << t << " ms\n";
        }
        else if (choice == 7) {
            s.compareSolvers();
        }
    }

    return 0;
}
