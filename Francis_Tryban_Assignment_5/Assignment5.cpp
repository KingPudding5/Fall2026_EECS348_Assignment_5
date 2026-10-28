//EECS 348 Assignment 4
//Sudoku Solver that use a brute force recursive algorithm
//input: puzzle1.txt through puzzle5.txt
//Output: solved puzzles
//Collaborators: Me
//Other Collaborators: Claude
//Weston G. Uptmor
//10/15/2026


#include <array> //used for grid
#include <cctype> //checks whether a character is a digit
#include <fstream> //for reading files
#include <iostream> //for output
#include <iterator> //reads a whole file as a string
#include <string> //allows the use of strings
#include <vector> //allows a list of filenames


class SudokuBoard { //initializes class SudokuBoard responsible for the board and the rules of placement
    public:
        static const int N = 9;      // board is N x N
        static const int B = 3;      // block is B x B
        static const int EMPTY = 0;  // internal marker for a blank square

        SudokuBoard() { grid_.fill(EMPTY); } //constructor that starts with every cell blank
        //following code reads puzzle from a file and returns it
        //code returns false if the file is invalid
        bool load(const std::string& filename) { //method "load"
            std::ifstream in(filename); //opens file
            if (!in) return false; //if file cannot be opened return false
            std::string text((std::istreambuf_iterator<char>(in)), //read entire file into one string
                            std::istreambuf_iterator<char>());
            int count = 0; //initializes the count of cells that have been read to 0
            for (char c : text) { //for loop that looks at each character in the file
                if (count >= N * N) { //if grid has been fully read
                    if (std::isdigit(static_cast<unsigned char>(c)) || c == '_') //if there are too many cells
                        return false;  // return false
                    continue; //ignore whitespace and other characters
                }
                if (c >= '1' && c <= '9')            grid_[count++] = c - '0'; //if a digit is already given, convert it to an integer and store it
                else if (c == '_' || c == '.' || c == '0') grid_[count++] = EMPTY; //otherwise store it as a blank space
                // all newlines and spaces are skipped
            }
            return count == N * N; //success if exactly 81 cells were read
        }

        int  get(int row, int col) const { return grid_[row * N + col]; } //read a cell
        void set(int row, int col, int v) { grid_[row * N + col] = v; } //write a value into said cell
        bool isEmpty(int row, int col) const { return get(row, col) == EMPTY; } //return true if cell is blank

        bool canPlace(int row, int col, int v) const { //initializes method canPlace that checks if num can be place in a given spot
            for (int i = 0; i < N; ++i) { //for loop that iterates along a row and a col together
                if (get(row, i) == v) return false;//if v already appears in this row, return false
                if (get(i, col) == v) return false;//if v already appears in this collumn, return false
            }
            int br = (row / B) * B, bc = (col / B) * B; //top left corner of a cell's 3x3 block
            for (int r = br; r < br + B; ++r) //iterates over block's rows
                for (int c = bc; c < bc + B; ++c) //iterates over block's column's
                    if (get(r, c) == v) return false; //if v already appears in this block, return false
            return true;//if placement is good, return True
        }

        bool givensAreConsistent() const { //initializes method "givensAreConsistent" which checks if the puzzle is unsolvable
            SudokuBoard tmp = *this; //creates a copy of the current board
            for (int r = 0; r < N; ++r) //iterates over rows
                for (int c = 0; c < N; ++c) { //iterates over columns
                    int v = tmp.get(r, c); //digit in the current cell
                    if (v == EMPTY) continue; //if the current cell is a blank, go to next one
                    tmp.set(r, c, EMPTY); //blanks out current cell
                    bool ok = tmp.canPlace(r, c, v); //checks if it would be legal to put current cell back
                    tmp.set(r, c, v); //puts the digit back
                    if (!ok) return false; //if there is a conflict, returns false
                }
            return true; //if there were no conflicts, returns true
    }

    void print(std::ostream& os) const { //initializes method "print" which prints out the board
        for (int r = 0; r < N; ++r) {//iterates rows
            if (r > 0 && r % B == 0) os << "------+-------+------\n";//adds a divider between 3x3 block rows
            for (int c = 0; c < N; ++c) {//iterates columns
                if (c > 0 && c % B == 0) os << "| ";//adds a divider between 3x3 block columns
                os << (isEmpty(r, c) ? '_' : static_cast<char>('0' + get(r, c)));//prints "_" for blanks and the number that's supposed to be there otherwise
                os << (c < N - 1 ? " " : "");//adds a space between cells
            }
            os << '\n';//adds a return line after each row
        }
    }

    private:
        std::array<int, N * N> grid_;//where the board is stored
};

// ---------------------------------------------------------------------------
// SudokuSolver: exhaustive depth-first search with backtracking.
// Works on its own copy of the board and collects every solution found.
// ---------------------------------------------------------------------------
class SudokuSolver { //initializes class SudokuSolver responsible for the search, collects every solution found
    public:
    //changed sodoku solver so now it takes the output stream and can immediately prints references which are the solutions
    SudokuSolver(const SudokuBoard& puzzle, std::ostream& os, std::size_t maxSolutions = 1000)// constructor, sets maximum solutions to 1000. This line and the next have been changed and authored by me and Claude
        : board_(puzzle), os_(os), maxSolutions_(maxSolutions) {} //copies puzzleboard and stores the stream and limit


    std::size_t solveAll() {//method solveAll that returns # of solutions found, method has been changed and is authored by me
        count_ = 0; //resets counter
        if (!board_.givensAreConsistent()) return 0; //contradictory givens
        search(0); //start search at first cell
        return count_;//returns the number of solutions found
    }


    private:
        SudokuBoard board_;//copy of board
        std::ostream& os_; //added as a location to be streamed to. This and following two lines have been authored by me
        std::size_t count_ = 0; //added count
        std::size_t maxSolutions_; //added max amount of solutions, can be changed depending on if you want more or less 

        void search(int pos) {//method search that is the recursive step which searches for solutions
            if (maxSolutions_ != 0 && count_ >= maxSolutions_) { //max amount of solutions enforcement. If statement authored by me
                return;//stops searching once max solutions have been reached
            }
            const int total = SudokuBoard::N * SudokuBoard::N;//total num of cells

            while (pos < total && !board_.isEmpty(pos / SudokuBoard::N, //convert pos to row
                                                pos % SudokuBoard::N)) //conver pos to col
                ++pos;//move to next cell

            // Every cell is filled: the solution is complete now and doesn't need to be stored
            if (pos == total) {// if solution is complete. If statement authored by me
                ++count_; //counter goes up one
                os_ << "\nSolution " << count_ << ":\n"; board_.print(os_); //prints solution without storing anything
                return;//go back and find more solutions
            }

            int row = pos / SudokuBoard::N;//row of current empty cell
            int col = pos % SudokuBoard::N;//column of current empty cell

            for (int v = 1; v <= SudokuBoard::N; ++v) {//iterate 1-9
                if (board_.canPlace(row, col, v)) { //if the digit is allowed there
                    board_.set(row, col, v);//try the digit there
                    search(pos + 1);//recurse
                    board_.set(row, col, SudokuBoard::EMPTY); //done recursing, go back
                }
            }
        }
};

class PuzzleRunner { //class PuzzleRunner that loads the file, solves the puzzle, and then prints everything out
    public:
        explicit PuzzleRunner(const std::string& filename) : filename_(filename) {} //stores file

        void run(std::ostream& os) {//method run that solve the puzzle and then prints it out
            os << "==========================================\n";//seperator between each puzzle
            os << "Puzzle file: " << filename_ << "\n\n";//prints name of puzzle file

            SudokuBoard puzzle;//empty board
            if (!puzzle.load(filename_)) {//if the puzzle cannot load
                os << "Error: could not read a valid 9x9 puzzle from this file.\n\n";//print that there was an error
                return;//exit method
            }

            os << "Puzzle:\n";//prints "Puzzle:" with a return line
            puzzle.print(os);//prints the unsolved puzzle

            SudokuSolver solver(puzzle, os); //creates an instance of Sudoku Solver called solver, updated to include os
            std::size_t n = solver.solveAll();//calls solveAll which solves the puzzle and prints it

            os << '\n';//blank line
            if (n == 0) {//if there are no solutions
                os << "no solution found\n\n";//print that there aren't any solutions
                return;//exit method
            }
            else{ //if there are solutions
                os << n << (n==1 ? " solution" : " solutions") << " found.\n\n"; //print the number of solutions, can now just print the amount of solutions instead of using a for loop
            }
        }

    private:
        std::string filename_;//name of the file
    };

    // ---------------------------------------------------------------------------
    int main(int argc, char* argv[]) {//main
        std::vector<std::string> files; //list of puzzle filenames
        if (argc > 1) { //if there are filenames given on command line
            for (int i = 1; i < argc; ++i) files.push_back(argv[i]);//use each filename
        } else { //otherwise, use default puzzles
            for (int i = 1; i <= 5; ++i)//iterate 1-5
                files.push_back("puzzle" + std::to_string(i) + ".txt");//puzzle1.txt - puzzle5.txt
        }

        for (const std::string& f : files) {//iterate through puzzles
            PuzzleRunner runner(f); //create a PuzzleRunner runner for this file
            runner.run(std::cout); //solves puzzle and prints results
        }
        return 0;//exit program
}