#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <array>
#include <random>

#define X 'X'
#define O 'O'
#define D 'D'
#define EMPTY ' '
#define SEQUENTIAL "sequential"
#define RANDOM "random"

// Classe TicTacToe
class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
    std::mutex board_mutex; // Mutex para controle de acesso ao tabuleiro
    std::condition_variable turn_cv; // Variável de condição para alternância de turnos
    char current_player; // Jogador atual ('X' ou 'O')
    bool game_over; // Estado do jogo
    char winner; // Vencedor do jogo

public:
    TicTacToe() {
        // Inicializar o tabuleiro e as variáveis do jogo
        this->board = {
        std::array<char, 3>{EMPTY, EMPTY, EMPTY},
        std::array<char, 3>{EMPTY, EMPTY, EMPTY},
        std::array<char, 3>{EMPTY, EMPTY, EMPTY}};
        this->current_player = X;
        this->game_over = false;
        this->winner = EMPTY;
    }

    void display_board();

    bool make_move(char player, int row, int col);

    bool check_win(char player);

    bool check_draw();

    bool is_game_over() {
        // Retornar se o jogo terminou
        return this->game_over;
    }

    char get_winner() {
        // Retornar o vencedor do jogo ('X', 'O', ou 'D' para empate)
        return this->winner;
    }
};

void TicTacToe::display_board() {
    // Exibir o tabuleiro no console
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf(" %c ", this->board[i][j]);
            if (j < 2) printf("|");                
        }
        printf("\n");
        if (i < 2) printf("-----------\n");
    }
}

bool TicTacToe::make_move(char player, int row, int col) {
    // Implementar a lógica para realizar uma jogada no tabuleiro
    // Utilizar mutex para controle de acesso
    // Utilizar variável de condição para alternância de turnos
    std::unique_lock<std::mutex> board_lock(this->board_mutex);

    while (this->current_player != player) {
        this->turn_cv.wait(board_lock);
    }
    
    if (this->board[row][col] == EMPTY && !this->is_game_over()) {
        this->board[row][col] = player; // Realiza a jogada

        system("cls");
        this->display_board();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        if (this->check_win(player)) {
            this->game_over = true;
            this->winner = player;
        } else {
            if (this->check_draw()) {
                this->game_over = true;
                this->winner = D;
            }
        }
        
        this->current_player = (player == X) ? O : X; // Troca o turno para o outro jogador
        this->turn_cv.notify_one();
        return true;
    }
    return false;
}

bool TicTacToe::check_win(char player) {
    // Verificar se o jogador atual venceu o jogo
    // Verifica linhas
    for (int i = 0; i < 3; i++) {
        if (this->board[i][0] == player && this->board[i][1] == player && this->board[i][2] == player) {
            return true;
        }
    }
    
    // Verifica colunas
    for (int i = 0; i < 3; i++) {
        if (this->board[0][i] == player && this->board[1][i] == player && this->board[2][i] == player) {
            return true;
        }
    }
    
    // Verifica diagonais
    if (this->board[0][0] == player && this->board[1][1] == player && this->board[2][2] == player) {
        return true;
    }
    if (this->board[0][2] == player && this->board[1][1] == player && this->board[2][0] == player) {
        return true;
    }
    
    // Retorna false se o jogador não venceu
    return false;
}

bool TicTacToe::check_draw() {
    // Verificar se houve um empate
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (this->board[i][j] == EMPTY) {
                return false;
            }
        }
    }
    return !this->check_win(X) && !this->check_win(O);
}

// Classe Player
class Player {
private:
    TicTacToe& game; // Referência para o jogo
    char symbol; // Símbolo do jogador ('X' ou 'O')
    std::string strategy; // Estratégia do jogador

public:
    Player(TicTacToe& g, char s, std::string strat) 
        : game(g), symbol(s), strategy(strat) {

            if (s != X && s != O){
                printf("Unknown player symbol\n");
            }

            if (strat != SEQUENTIAL && strat != RANDOM){
                printf("Unknown strategy\n");
            }
        }

    void play();

private:
    void play_sequential();

    void play_random();
};

void Player::play() {
    // Executar jogadas de acordo com a estratégia escolhida
    if (this->strategy == SEQUENTIAL) {
        this->play_sequential();
    } else {
        this->play_random();
    }        
}

void Player::play_sequential() {
    // Implementar a estratégia sequencial de jogadas
    for (int i = 0; i < 3 && !this->game.is_game_over(); i++) {
        for (int j = 0; j < 3 && !this->game.is_game_over(); j++) {
            this->game.make_move(this->symbol, i, j);
        }
    }
}

void Player::play_random() {
    // Implementar a estratégia aleatória de jogadas

    // Inicializa o gerador de números aleatórios com uma semente aleatória
    std::mt19937 gen(static_cast<unsigned int>(std::time(0)));

    // Define o intervalo de valores (distribuição uniforme)
    std::uniform_int_distribution<int> distrib(0, 2);

    int randomRow, randomCol;

    while (!this->game.is_game_over()){
        // Gera linhas e colunas aleatórias no intervalo [0, 2]
        int randomRow = distrib(gen);
        int randomCol = distrib(gen);
        this->game.make_move(this->symbol, randomRow, randomCol);
    }
}

void play(Player& player) {
    player.play();
}

// Função principal
int main() {
    // Inicializar o jogo e os jogadores
    TicTacToe game;
    Player player1(game, X, SEQUENTIAL);
    Player player2(game, O, RANDOM);

    // Criar as threads para os jogadores
    std::thread pl1(play, std::ref(player1));
    std::thread pl2(play, std::ref(player2));

    // Aguardar o término das threads
    pl1.join();
    pl2.join();

    // Exibir o resultado final do jogo
    system("cls");
    game.display_board();

    if (game.get_winner() == X) {
        printf("O jogador 'X' venceu\n");
    } else if (game.get_winner() == O) {
        printf("O jogador 'O' venceu\n");
    } else {
        printf("Deu velha\n");
    }

    return 0;
}
