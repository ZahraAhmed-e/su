#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <chrono>

using namespace std;
using namespace std::chrono;

class Sudoku {
private:
    int grid[9][9];
    int original[9][9];

public:
    int stepsDFS = 0, backtracksDFS = 0;
    int stepsFC = 0, backtracksFC = 0;
    int stepsAC3 = 0, backtracksAC3 = 0;

    Sudoku() {
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                grid[r][c] = 0, original[r][c] = 0;
    }

    void printUI() const {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        cout << "\n=========== SUDOKU SOLVER ===========\n";
        cout << " Backtracking + Forward Checking + AC-3\n";
        cout << "======================================\n\n";

        for (int r = 0; r < 9; r++) {
            if (r % 3 == 0) cout << "-----------------------------------------\n";
            for (int c = 0; c < 9; c++) {
                if (c % 3 == 0) cout << "| ";
                if (grid[r][c] == 0) cout << ". ";
                else cout << grid[r][c] << " ";
            }
            cout << "|\n";
        }
        cout << "-----------------------------------------\n\n";
    }

    void readFromUser() {
        cout << "أدخل لوحة سودوكو (0 = خانة فارغة):\n";
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                cin >> grid[r][c];
    }

    bool checkRow(int row, int val) const {
        for (int c = 0; c < 9; c++)
            if (grid[row][c] == val) return false;
        return true;
    }

    bool checkCol(int col, int val) const {
        for (int r = 0; r < 9; r++)
            if (grid[r][col] == val) return false;
        return true;
    }

    bool checkBox(int row, int col, int val) const {
        int sr = (row / 3) * 3;
        int sc = (col / 3) * 3;
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 3; c++)
                if (grid[sr + r][sc + c] == val) return false;
        return true;
    }

    bool isSafe(int row, int col, int val) const {
        return checkRow(row, val) && checkCol(col, val) && checkBox(row, col, val);
    }

    bool findEmpty(int& row, int& col) const {
        for (row = 0; row < 9; row++)
            for (col = 0; col < 9; col++)
                if (grid[row][col] == 0) return true;
        return false;
    }

    vector<int> getPossibleValues(int row, int col) const {
        vector<int> vals;
        if (grid[row][col] != 0) return vals;
        for (int v = 1; v <= 9; v++)
            if (isSafe(row, col, v))
                vals.push_back(v);
        return vals;
    }

    void copyOriginal() {
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                original[r][c] = grid[r][c];
    }

    void restoreOriginal() {
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                grid[r][c] = original[r][c];
    }

    void printBoard() const {
        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 9; c++) {
                cout << grid[r][c] << " ";
            }
            cout << "\n";
        }
    }

    bool solveBacktracking() {
        int row, col;
        if (!findEmpty(row, col)) return true;

        for (int num = 1; num <= 9; num++) {
            stepsDFS++;
            if (isSafe(row, col, num)) {
                grid[row][col] = num;
                if (solveBacktracking()) return true;
                grid[row][col] = 0;
                backtracksDFS++;
            }
        }
        return false;
    }

    bool forwardCheckBoard() const {
        for (int r = 0; r < 9; r++)
            for (int c = 0; c < 9; c++)
                if (grid[r][c] == 0 && getPossibleValues(r, c).empty())
                    return false;
        return true;
    }

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

    vector<vector<vector<int>>> initDomains() const {
        vector<vector<vector<int>>> domains(9, vector<vector<int>>(9));
        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 9; c++) {
                if (grid[r][c] != 0) domains[r][c] = {grid[r][c]};
                else domains[r][c] = getPossibleValues(r, c);
            }
        }
        return domains;
    }

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

    bool ac3(vector<vector<vector<int>>>& domains) const {
        queue<pair<pair<int,int>, pair<int,int>>> q;

        for (int r1 = 0; r1 < 9; r1++) {
            for (int c1 = 0; c1 < 9; c1++) {
                for (int r2 = 0; r2 < 9; r2++) {
                    for (int c2 = 0; c2 < 9; c2++) {
                        if (r1 == r2 && c1 == c2) continue;
                        bool sameRow = (r1 == r2);
                        bool sameCol = (c1 == c2);
                        bool sameBox = (r1 / 3 == r2 / 3 && c1 / 3 == c2 / 3);
                        if (sameRow || sameCol || sameBox) {
                            q.push({{r1, c1}, {r2, c2}});
                        }
                    }
                }
            }
        }

        while (!q.empty()) {
            auto arc = q.front();
            q.pop();

            int xi_r = arc.first.first;
            int xi_c = arc.first.second;
            int xj_r = arc.second.first;
            int xj_c = arc.second.second;

            if (revise(domains, xi_r, xi_c, xj_r, xj_c)) {
                if (domains[xi_r][xi_c].empty()) return false;

                for (int r = 0; r < 9; r++) {
                    for (int c = 0; c < 9; c++) {
                        if (r == xi_r && c == xi_c) continue;
                        if (r == xj_r && c == xj_c) continue;

                        bool sameRow = (r == xi_r);
                        bool sameCol = (c == xi_c);
                        bool sameBox = (r / 3 == xi_r / 3 && c / 3 == xi_c / 3);
                        if (sameRow || sameCol || sameBox) {
                            q.push({{r, c}, {xi_r, xi_c}});
                        }
                    }
                }
            }
        }

        return true;
    }

    bool solveAC3() {
        auto domains = initDomains();
        if (!ac3(domains)) return false;
        return solveAC3Recursive(domains);
    }

    bool solveAC3Recursive(vector<vector<vector<int>>> domains) {
        int row = -1, col = -1, minSize = 10;

        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 9; c++) {
                if (grid[r][c] == 0) {
                    int sz = domains[r][c].size();
                    if (sz < minSize) {
                        minSize = sz;
                        row = r;
                        col = c;
                    }
                }
            }
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

    void compareSolvers() {
        copyOriginal();

        stepsDFS = backtracksDFS = 0;
        auto start = high_resolution_clock::now();
        bool solvedDFS = solveBacktracking();
        auto end = high_resolution_clock::now();
        auto timeDFS = duration_cast<milliseconds>(end - start).count();

        cout << "\nBacktracking:\n";
        cout << "  حل = " << (solvedDFS ? "نعم" : "لا") << "\n";
        cout << "  عدد العقد = " << stepsDFS << "\n";
        cout << "  عدد التراجعات = " << backtracksDFS << "\n";
        cout << "  وقت الحل = " << timeDFS << " ms\n";

        restoreOriginal();

        stepsFC = backtracksFC = 0;
        start = high_resolution_clock::now();
        bool solvedFC = solveForwardChecking();
        end = high_resolution_clock::now();
        auto timeFC = duration_cast<milliseconds>(end - start).count();

        cout << "\nForward Checking:\n";
        cout << "  حل = " << (solvedFC ? "نعم" : "لا") << "\n";
        cout << "  عدد العقد = " << stepsFC << "\n";
        cout << "  عدد التراجعات = " << backtracksFC << "\n";
        cout << "  وقت الحل = " << timeFC << " ms\n";

        restoreOriginal();

        stepsAC3 = backtracksAC3 = 0;
        start = high_resolution_clock::now();
        bool solvedA = solveAC3();
        end = high_resolution_clock::now();
        auto timeA = duration_cast<milliseconds>(end - start).count();

        cout << "\nAC-3:\n";
        cout << "  حل = " << (solvedA ? "نعم" : "لا") << "\n";
        cout << "  عدد العقد = " << stepsAC3 << "\n";
        cout << "  عدد التراجعات = " << backtracksAC3 << "\n";
        cout << "  وقت الحل = " << timeA << " ms\n";

        restoreOriginal();
    }
};

int main() {
    Sudoku s;
    int choice;

    while (true) {
        s.printUI();
        cout << "1) إدخال لوحة سودوكو\n";
        cout << "2) حل Backtracking\n";
        cout << "3) مسح اللوحة\n";
        cout << "4) خروج\n";
        cout << "5) حل Forward Checking\n";
        cout << "6) حل AC-3\n";
        cout << "7) مقارنة الخوارزميات\n";
        cout << "اختر: ";
        cin >> choice;

        if (choice == 1) {
            s.readFromUser();
            s.copyOriginal();
        }
        else if (choice == 2) {
            s.stepsDFS = s.backtracksDFS = 0;
            if (s.solveBacktracking()) cout << "تم الحل بنجاح.\n";
            else cout << "لا يوجد حل.\n";
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
            if (s.solveForwardChecking()) cout << "تم الحل بنجاح.\n";
            else cout << "لا يوجد حل.\n";
        }
        else if (choice == 6) {
            s.restoreOriginal();
            s.stepsAC3 = s.backtracksAC3 = 0;
            if (s.solveAC3()) cout << "تم الحل بنجاح.\n";
            else cout << "لا يوجد حل.\n";
        }
        else if (choice == 7) {
            s.compareSolvers();
        }
    }

    return 0;
}