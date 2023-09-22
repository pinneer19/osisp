#include <unordered_set>
#include <ctime>
#include <cstdlib>
using namespace std;
enum Mode {
	BEGINNER, ADVANCED, EXPERT
};
struct hashFunction
{
	size_t operator()(const std::pair<short, short>& x) const {
		return x.first ^ x.second;
	}
};

class Minesweeper {

public:
	Minesweeper(Mode mode = BEGINNER) {
		switch (mode)
		{
		case BEGINNER:
			fieldSize = 9;
			break;
		case ADVANCED:
			fieldSize = 14;
			break;
		case EXPERT:
			fieldSize = 20;
			break;
		}
		opened_cells = fieldSize * fieldSize;
		setField();
		setMines();
	}

	short getFieldSize() {
		return fieldSize;
	}

	wchar_t getStatus(short i, short j) {
		return field[i][j];
	}

	wchar_t setAsMine(short i, short j) {
		switch (field[i][j]) {
		case '.':
			field[i][j] = L'*';
			opened_cells--;
			break;
		case L'*':
			opened_cells++;
			field[i][j] = '.';
			break;
		}
		return field[i][j];
	}

	bool checkWinner() {
		return opened_cells == 0;
	}

	bool openCell(short i, short j) {
		if (mines.contains(std::pair(i, j))) {
			if (first) {
				mines.erase(std::pair(i, j));
				first = false;
				return true;
			}
			else return false;
		}
		first = false;
		return true;
	}

	Mode getMode() {
		switch (fieldSize) {
		case 9:
			return BEGINNER;
			break;
		case 14:
			return ADVANCED;
			break;
		case 20:
			return EXPERT;
			break;
		}
	}

	wchar_t checkCell(short i, short j) {
		if (field[i][j] != '.' && field[i][j] != L'*') return field[i][j];
		int counter = 0;
		for (short x = max(i - 1, 0); x <= min(i + 1, fieldSize - 1); ++x) {
			for (short y = max(j - 1, 0); y <= min(j + 1, fieldSize - 1); ++y) {
				if (mines.contains(pair(x, y))) counter++;
			}
		}

		if (counter == 0) {
			field[i][j] = L'/';	
		}
		else {
			field[i][j] = counter + '0';
		}
		opened_cells--;
		return field[i][j];
	}
	
private:
	short fieldSize;
	wchar_t** field;
	bool first = true;
	int opened_cells;
	std::unordered_set<std::pair<short, short>, hashFunction> mines;

	void setField() {
		field = new wchar_t* [fieldSize];
		for (int i = 0; i < fieldSize; ++i) {
			field[i] = new wchar_t[fieldSize];
			for (int j = 0; j < fieldSize; ++j) {
				field[i][j] = '.'; // Set the default value as dot '.'
			}
		}
	}

	void setMines() {
		srand(time(NULL));
		if(!mines.empty()) mines.clear();
		while (mines.size() < fieldSize) {

			short x = static_cast<short>(rand() % fieldSize);
			short y = static_cast<short>(rand() % fieldSize);

			std::pair<short, short> mine(x, y);

			if (!mines.contains(mine)) {
				mines.insert(mine);
			}
		}
	}
};