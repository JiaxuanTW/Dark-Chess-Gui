#include <QTime>
#include <QMouseEvent>
#include <QThread>

#include "SinglePlayer.h"
#include "MainWindow.h"
#include "ResultScreen.h"

QVector<NextMoveInfo> SinglePlayer::generateMoves() {
	QVector<NextMoveInfo> nextMoveInfoVector;
	if (turn == RED) {
		for (size_t i = 1; i <= 7; i++) {
			unsigned int p = chessType[i].pos & (~chessType[15].pos); // 找到該類棋所有位置 - 暗子
			while (p) {
				unsigned int check = black | chessType[0].pos;
				unsigned int mask = ChessType::getLSB(p);
				unsigned int dest = 0x00000000;
				p ^= mask;
				int srcSquareId = getSquareId(mask);
				switch (i) {
				case 1:
					dest = moveMask[srcSquareId] & (check ^ chessType[14].pos);
					break;
				case 2:
					dest = moveMask[srcSquareId] & (check ^ chessType[8].pos);
					break;
				case 3:
					dest = moveMask[srcSquareId] & (check ^ chessType[8].pos ^ chessType[9].pos);
					break;
				case 4:
					dest = moveMask[srcSquareId] & (chessType[0].pos | chessType[11].pos |
						chessType[12].pos | chessType[13].pos | chessType[14].pos);
					break;
				case 5:
					dest = moveMask[srcSquareId] & (chessType[0].pos | chessType[12].pos |
						chessType[13].pos | chessType[14].pos);
					break;
				case 6:
					dest = ((Cgen(srcSquareId) & black) | (moveMask[srcSquareId] & chessType[0].pos));
					break;
				case 7:
					dest = moveMask[srcSquareId] & (chessType[0].pos | chessType[8].pos |
						chessType[14].pos);
					break;
				}

				dest &= (~chessType[15].pos); // 得到的位置必須扣掉暗子
				while (dest) {
					unsigned int mask2 = ChessType::getLSB(dest);
					dest ^= mask2;
					int destSquareId = getSquareId(mask2);
					NextMoveInfo nextMoveInfo(srcSquareId, destSquareId);
					nextMoveInfoVector.append(nextMoveInfo);
					//cout << aTurn << "  - generateMove: " << nextMoveInfoVector.last().src << ", " << nextMoveInfoVector.last().dest << endl;
				}
			}
		}
	}
	else {
		for (size_t i = 8; i <= 14; i++) {
			unsigned int p = chessType[i].pos & (~chessType[15].pos); // 找到該類棋所有位置 - 暗子
			while (p) {
				unsigned int check = red | chessType[0].pos;
				unsigned int mask = ChessType::getLSB(p);
				unsigned int dest;
				p ^= mask;
				int srcSquareId = getSquareId(mask);

				switch (i) {
				case 8:
					dest = moveMask[srcSquareId] & (check ^ chessType[7].pos);
					break;
				case 9:
					dest = moveMask[srcSquareId] & (check ^ chessType[1].pos);
					break;
				case 10:
					dest = moveMask[srcSquareId] & (check ^ chessType[1].pos ^ chessType[2].pos);
					break;
				case 11:
					dest = moveMask[srcSquareId] & (chessType[0].pos | chessType[4].pos |
						chessType[5].pos | chessType[6].pos | chessType[7].pos);
					break;
				case 12:
					dest = moveMask[srcSquareId] & (chessType[0].pos | chessType[5].pos |
						chessType[6].pos | chessType[7].pos);
					break;
				case 13:
					dest = ((Cgen(srcSquareId) & red) | (moveMask[srcSquareId] & chessType[0].pos));
					break;
				case 14:
					dest = moveMask[srcSquareId] & (chessType[0].pos | chessType[1].pos |
						chessType[7].pos);
					break;
				}

				dest &= (~chessType[15].pos); // 得到的位置必須扣掉暗子
				while (dest) {
					unsigned int mask2 = ChessType::getLSB(dest);
					dest ^= mask2;
					int destSquareId = getSquareId(mask2);
					NextMoveInfo nextMoveInfo(srcSquareId, destSquareId);
					nextMoveInfoVector.append(nextMoveInfo);
					//cout << aTurn <<  "  - generateMove: " << nextMoveInfoVector.last().src << ", " << nextMoveInfoVector.last().dest << endl;
				}
			}
		}
	}
	return nextMoveInfoVector;
}

SinglePlayer::SinglePlayer() {
	setPlayerInfo("PLAYER", "COMPUTER");
}

PiecePosition SinglePlayer::saveCurrentBoard() {
	// 儲存現在棋盤面
	// 回傳棋盤面

	PiecePosition piecePosition = PiecePosition();
	for (size_t i = 0; i < 16; i++)
		piecePosition.chessType[i].pos = chessType[i].pos;

	piecePosition.red = red;
	piecePosition.black = black;
	piecePosition.occupied = occupied;
	return piecePosition;
}

void SinglePlayer::computerMove() {
	QThread::sleep(1);
	QVector<NextMoveInfo> nextMoveInfo = generateMoves();

	// 如果沒有可移動步伐，就進行翻棋
	if (nextMoveInfo.isEmpty() && chessType[15].pos) {
		printf("I AM LEO.");
		qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime())); // 初始化 qsrand 種子
		int randomPiece = 0;
		do {
			randomPiece = qrand() % 32;
		} while (rowMask[getRowId(randomPiece)] &
			colMask[getColumnId(randomPiece)] & (~chessType[15].pos));
		flipPiece(randomPiece);
		return;
	}

	int maxScore = INT_MIN;
	NextMoveInfo bestMove = NextMoveInfo(0, 0);
	while (!nextMoveInfo.isEmpty()) {
		NextMoveInfo nextMove = nextMoveInfo.back();
		nextMoveInfo.removeLast();

		PiecePosition currentBoard = saveCurrentBoard();
		movePiece(nextMove.src, nextMove.dest);// HACK: 換邊問題
		int score = miniMax(4, negaTurn(turn), INT_MIN, INT_MAX);
		undoFakeMove(currentBoard);

		if (score > maxScore) {
			maxScore = score;
			bestMove = nextMove;
		}
		/*cout << "COMPUTER_MOVE:: turn:"<< turn << " value:" << score << ", best:" << maxScore
			<< " at MOVE " << nextMove.src << "->" << nextMove.dest << endl;*/
	}
	/*cout << stepsCounter << " - bestMove:" << bestMove.src << "->" <<
		bestMove.dest << " - maxScore:" << maxScore << endl;*/
	if (maxScore <= evaluate() && chessType[15].pos) {
		qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime())); // 初始化 qsrand 種子
		int randomPiece = 0;
		do {
			randomPiece = qrand() % 32;
		} while (rowMask[getRowId(randomPiece)] &
			colMask[getColumnId(randomPiece)] & (~chessType[15].pos));
		flipPiece(randomPiece);
	}
	else {
		movePiece(bestMove.src, bestMove.dest);
	}
}

int SinglePlayer::evaluate() {
	int pieceWeight[7] = { 810, 270, 90, 18, 6, 18, 1 }; // 權重
	int redValue = 0;
	int blackValue = 0;

	unsigned int redCopy = red;
	while (redCopy) {
		unsigned int mask = ChessType::getLSB(redCopy);
		redCopy ^= mask;

		int chessTypeId = getChessTypeId(getSquareId(mask));
		if (chessTypeId > 7 || chessTypeId < 1) {
			printf(">>>>> ERROR: turn: RED\n");
			printf(">>>>> ERROR: chessTypeId: %d\n", chessTypeId);
			printf(">>>>> ERROR: red: %08x\n", red);
			printf(">>>>> ERROR: redCopy: %08x\n", redCopy);
			printf(">>>>> ERROR: mask: %08x\n", mask);
			printf(">>>>> ERROR: squareId: %d\n", getSquareId(mask));
		}
		redValue += pieceWeight[chessTypeId - 1];

		/*if (pieceWeight[chessTypeId - 1] > 5000 || pieceWeight[chessTypeId - 1] < -5000) {
			cout << "EVALUATE:: red[" << getChessTypeId(getSquareId(mask)) <<
				"] add " << pieceWeight[getChessTypeId(getSquareId(mask)) - 1] <<
				" at Square " << getSquareId(mask) << endl;
		}*/
	}

	unsigned int blackCopy = black;
	while (blackCopy) {
		unsigned int mask = ChessType::getLSB(blackCopy);
		blackCopy ^= mask;

		int chessTypeId = getChessTypeId(getSquareId(mask));
		if (chessTypeId > 14 || chessTypeId < 8) {
			printf(">>>>> ERROR: turn: BLACK\n");
			printf(">>>>> ERROR: chessTypeId: %d\n", chessTypeId);
			printf(">>>>> ERROR: black: %08x\n", black);
			printf(">>>>> ERROR: blackCopy: %08x\n", blackCopy);
			printf(">>>>> ERROR: mask: %08x\n", mask);
			printf(">>>>> ERROR: squareId: %d\n", getSquareId(mask));
		}
		blackValue += pieceWeight[chessTypeId - 8];

		/*if (pieceWeight[chessTypeId - 8] > 5000 || pieceWeight[chessTypeId - 8] < -5000) {
			cout << "EVALUATE:: black[" << getChessTypeId(getSquareId(mask)) <<
				"] add " << pieceWeight[getChessTypeId(getSquareId(mask)) - 8] <<
				" at Square " << getSquareId(mask) << endl;
		}*/
	}

	if (turn == BLACK)
		return redValue - blackValue;
	else
		return blackValue - redValue;
}

int SinglePlayer::miniMax(int depth, int aTurn, int alpha, int beta) {
	//cout << "Enter miniMax depth: " << depth << ", turn: " << turn << endl;
	QVector<NextMoveInfo> childNodes = generateMoves();
	int best;
	if (turn == aTurn) {
		best = INT_MIN;
	}
	else {
		best = INT_MAX;
	}
	int value = 0;

	if (depth == 0 || childNodes.isEmpty()) {
		//cout << "Leave miniMax with returning evaluate()" << endl;
		return evaluate();
	}


	while (!childNodes.isEmpty()) {
		NextMoveInfo node = childNodes.back();
		childNodes.removeLast();

		PiecePosition board = saveCurrentBoard();
		movePiece(node.src, node.dest);

		value = miniMax(depth - 1, aTurn, alpha, beta);
		//cout << "value:" << value << endl;
		//cout << "first b : " << beta << "a :" << alpha << endl;
		undoFakeMove(board);

		if (turn == aTurn) {
			//cout << " himy " << endl;
			if (value >= alpha) {
				//	cout << " hi1 " << endl;
				alpha = value;
			}
			if (beta < alpha) {
				best = beta;
				//cout << " ff " << endl;
				break;
			}
			if (alpha > best) {
				best = alpha;
				//cout << " hi2 "<<best<<endl;
			}

		}
		else {
			//cout << " hinot " << endl;
			if (value <= beta) {
				//cout << " hi3 " << endl;
				beta = value;
			}
			if (beta < alpha) {
				best = alpha;
				//cout << " ff " << endl;
				break;
			}
			if (beta < best) {
				best = beta;
				//cout << " hi4 " <<best<< endl;
			}
		}
		//cout <<"best :"<<best<<"b : "<< beta<<" a: "<<alpha << endl;
	}
	//cout << "\n return best : " << best << endl;
	/*cout << "Leave miniMax depth: " << depth << endl;*/
	return best;
}

void SinglePlayer::undoFakeMove(PiecePosition piecePosition) {
	// 回復到指定局面
	// piecePostion: 棋盤面物件

	for (size_t i = 0; i < 16; i++)
		chessType[i].pos = piecePosition.chessType[i].pos;

	red = piecePosition.red;
	black = piecePosition.black;
	occupied = piecePosition.occupied;

	// 換邊
	nextTurn();
	stepCounter--;
	roundCounter = stepCounter / 2;
}

void SinglePlayer::mouseReleaseEvent(QMouseEvent* ev) {
	// 滑鼠點擊事件
	// ev: 滑鼠參數
	int clickedId = click(ev->pos());

	if (ev->x() >= 1530 && ev->x() <= 1600 && 
		ev->y() >= 0 && ev->y() <= 50) {
		MainWindow* control = static_cast<MainWindow*>(this->parentWidget()->parentWidget());
		control->redirectToScreen(1);
		init();
		setPlayerInfo("PLAYER", "COMPUTER");
		shuffle();
	}

	//若點擊非法範圍 -> 無效
	if (clickedId == -1)
		return;

	//若第一手 -> 翻棋並決定玩家棋方
	if (turn == NULL) {
		flipPiece(clickedId);
		getChessTypeId(clickedId) <= 7 ? turn = BLACK : turn = RED;
		getChessTypeId(clickedId) <= 7 ? turnConstant = BLACK : turnConstant = RED;
		repaint();
		computerMove();
		repaint();
		return;
	}

	//選子行為，第一次選子
	if (selectedId == -1) { 
		if (rowMask[getRowId(clickedId)] & colMask[getColumnId(clickedId)]
			& chessType[15].pos) {
			// 翻子
			flipPiece(clickedId);
			repaint();
			computerMove();
			//if (checkWinner()) {
			//	MainWindow* control = static_cast<MainWindow*>(this->parentWidget()->parentWidget());
			//	control->switchToResult(ResultScreen::ComputerWins);
			//}
		}
		else {
			selectedId = clickedId;
		}
	}
	else {
		if (validMove(selectedId, clickedId)) {
			movePiece(selectedId, clickedId);
			selectedId = -1;
			repaint();
			computerMove();
			//if (checkWinner()) {
			//	MainWindow* control = static_cast<MainWindow*>(this->parentWidget()->parentWidget());
			//	control->switchToResult(ResultScreen::ComputerWins);
			//}
		}
		else {
			printf("Capture Failed!\n");
		}
		selectedId = -1;
	}
	repaint();
}