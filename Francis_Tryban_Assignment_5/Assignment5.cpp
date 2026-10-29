//EECS 348 Assignment 5
//Solve Sudoku puzzles with recursive backtracking and save each puzzle's solutions.
//Input: puzzle1.txt through puzzle5.txt, or filenames given on the command line.
//Output: the original console report and solutionN.txt files with grids and a count.
//Original author: Weston G. Uptmor; creation date: 10/15/2026.
//Revised by: Francis Tryban; revision date: 10/28/2026.
//Collaborators: Weston credited Claude for his original program.
//Claude Haiku 4.5 supplied the first file-output changes used for this revision.
//OpenAI GPT-6.1-Sol in Zed helped Francis revise the code, comments and analysis.
//Unchanged code/comments are Weston's; their existing "me" credits refer to him.


#include <array> //used for grid
#include <cctype> //checks whether a character is a digit
#include <fstream> //for reading files
#include <iostream> //for output
#include <iterator> //reads a whole file as a string
#include <string> //allows the use of strings
#include <vector> //allows a list of filenames


//Weston's board and rule checks are retained below; the added file printer is marked A5.
class SudokuBoard { //initializes class SudokuBoard responsible for the board and the rules of placement
    public:
        static const int N = 9;      // board is N x N
        static const int B = 3;      // block is B x B
        static const int EMPTY = 0;  // internal marker for a blank square

        SudokuBoard() { grid_.fill(EMPTY); } //constructor that starts with every cell blank
        //Original loading code by Weston, with the Claude assistance credited in his header.
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

        //Weston's cell-access helpers: read a value, replace it, or check for a blank.
        int  get(int row, int col) const { return grid_[row * N + col]; } //read a cell
        void set(int row, int col, int v) { grid_[row * N + col] = v; } //write a value into said cell
        bool isEmpty(int row, int col) const { return get(row, col) == EMPTY; } //return true if cell is blank

        //Weston's placement check compares the candidate with its row, column and box.
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

        //Weston's clue check is retained; GPT-6.1-Sol clarified its description.
        bool givensAreConsistent() const { //checks for conflicting clues, not whether a solution must exist
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

    //Weston's original display printer keeps its box dividers for the console.
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

    //A5: Claude's file printer, revised with GPT-6.1-Sol to separate the cells.
    //Each row uses the same space-separated layout as the input puzzle files.
    void printRaw(std::ostream& os) const { //write the board without changing it
        for (int r = 0; r < N; ++r) { //visit the nine rows in order
            for (int c = 0; c < N; ++c) { //visit each cell in this row
                if (c > 0) os << ' '; //separate cells without adding box dividers
                os << (isEmpty(r, c) ? '_' : static_cast<char>('0' + get(r, c))); //write a blank or digit
            }
            os << '\n'; //finish this row
        }
        os << '\n'; //keep consecutive solutions easy to tell apart
    }

    private:
        std::array<int, N * N> grid_;//where the board is stored
};

//A5: GPT-6.1-Sol supplied the missing definition needed by array::fill.
const int SudokuBoard::EMPTY;

// ---------------------------------------------------------------------------
// SudokuSolver: exhaustive depth-first search with backtracking.
// Works on its own copy of the board and prints each solution as it is found.
// ---------------------------------------------------------------------------
//Weston's search is retained; Claude added file output, and GPT-6.1-Sol removed the default cap.
class SudokuSolver { //search the board and print each solution without collecting solved boards
    public:
    //A5: Claude added the file stream; GPT-6.1-Sol changed the default limit to 0.
    //Zero means no limit, so the normal run searches for every solution.
    SudokuSolver(const SudokuBoard& puzzle, std::ostream& os, std::ostream& solutionFile, std::size_t maxSolutions = 0)
        : board_(puzzle), os_(os), solutionFile_(solutionFile), maxSolutions_(maxSolutions) {} //copy the board and keep the stream references and chosen limit


    //Weston's original solve entry point checks the clues before starting the search.
    std::size_t solveAll() {//method solveAll that returns # of solutions found, method has been changed and is authored by me
        count_ = 0; //resets counter
        if (!board_.givensAreConsistent()) return 0; //contradictory givens
        search(0); //start search at first cell
        return count_;//returns the number of solutions found
    }


    private:
        SudokuBoard board_;//copy of board
        std::ostream& os_; //Weston's console stream
        std::ostream& solutionFile_; //A5: Claude's added stream for the solution file
        std::size_t count_ = 0; //added count
        std::size_t maxSolutions_; //added max amount of solutions, can be changed depending on if you want more or less

        //Weston's recursive search; Claude's added file write is marked at the completed board.
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
                board_.printRaw(solutionFile_); //A5: Claude also writes this solution to its file
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

//Weston's runner retains its console report; the A5 file-handling additions are marked below.
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

            //A5: Claude opened the solution file here; GPT-6.1-Sol added the check.
            std::string solutionFilename = getSolutionFilename(filename_); //match the input puzzle number
            std::ofstream solutionFile(solutionFilename); //create the file or replace its old contents
            if (!solutionFile) { //opening can fail, such as when the location is not writable
                os << "Error: could not create " << solutionFilename << ".\n\n"; //name the failed output
                return; //do not pretend the solutions were saved
            }

            SudokuSolver solver(puzzle, os, solutionFile); //A5: send solutions to both destinations
            std::size_t n = solver.solveAll();//calls solveAll which solves the puzzle and prints it

            os << '\n';//blank line
            if (n == 0) {//if there are no solutions
                os << "No solution found\n\n"; //A4 console message
                solutionFile << "No solution found.\n"; //A5: Claude's required file message
            }
            else{ //if there are solutions
                os << n << (n==1 ? " solution" : " solutions") << " found.\n\n"; //print the number of solutions, can now just print the amount of solutions instead of using a for loop
                solutionFile << "Solutions found: " << n << '\n'; //A5: Claude's file summary
            }
            //A5: GPT-6.1-Sol added a final check so write errors are reported too.
            solutionFile.close(); //finish pending writes before checking the stream
            if (!solutionFile) os << "Error: could not finish writing " << solutionFilename << ".\n\n";
        }

    private:
        std::string filename_;//name of the file

        //A5: Claude's filename helper, revised with GPT-6.1-Sol to find digits.
        //Ignore folder names, then use the first digit group in the input name.
        std::string getSolutionFilename(const std::string& filename) const { //build the output name
            std::size_t slash = filename.find_last_of("/\\"); //find the last folder separator, if any
            std::string base = slash == std::string::npos ? filename : filename.substr(slash + 1); //keep only the filename
            std::size_t start = base.find_first_of("0123456789"); //locate the first puzzle-number digit
            if (start == std::string::npos) return "solution.txt"; //fallback for unnumbered inputs
            std::size_t end = base.find_first_not_of("0123456789", start); //stop at the first non-digit
            //If the digit group reaches the end, substr keeps the remaining characters.
            return "solution" + base.substr(start, end - start) + ".txt"; //puzzle3.txt becomes solution3.txt
        }
    };

    // ---------------------------------------------------------------------------
    //Weston's original main selects input files and runs one PuzzleRunner for each.
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
