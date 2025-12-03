#include <vector>
#include <list>
#include <array>
#include <iostream>
#include <string>
#include <cstdint> 
#include <chrono>
#include <functional>

// 1. 'static': A matriz é criada apenas UMA vez na memória do programa (segmento de dados),
    //    não na stack e nem na heap a cada chamada.
    // 2. 'constexpr': Indica ao compilador que isso é constante absoluta.
    // 3. 1D Array: Usamos array linear de 25 posições para garantir layout contíguo
static constexpr std::array<int, 25> afinidade = {
     // P  N  A  B  T
        1, 3, 1, 3, 1, // P
        5, 1, 0, 1, 1, // N
        0, 1, 0, 4, 1, // A
        1, 3, 2, 3, 1, // B
        1, 1, 1, 1, 1  // T (Sempre 1)
    };



class Tabela_Afininidade {
    public:

    // Função "inline" implícita (definida na classe).
    // O 'noexcept' ajuda o compilador a otimizar, pois sabe que não haverá erros
    static constexpr int get(int typeA, int typeB) noexcept {
        return afinidade[typeA * 5 + typeB];
        // É uma row-major matrix, o calculo do indice é linha * ncols + col
    }
};


inline uint64_t calculate_energy(
    int p_left, int type_left,  // (i-1)
    int p_middle, int type_middle, // (i)
    int p_right, int type_right // (i+1)
) {

    uint64_t term1 = (uint64_t)p_left * Tabela_Afininidade::get(type_left, type_middle) * (uint64_t)p_middle;
    uint64_t term2 = (uint64_t)p_middle * Tabela_Afininidade::get(type_middle, type_right) * (uint64_t)p_right;

    return term1 + term2;
}

// Estrutura auxiliar para simular a recursão
struct Frame {
    int i;
    int j;
    bool visited; // false = expandir filhos; true = imprimir k
};

void optimal_path(
    int n,
    int dim,
    const std::vector<int>& ks,
    const std::function<int (int, int)>& idx
){
    std::vector <Frame> stack;

    if(n >= 1){
        stack.push_back({1, n, false});
    }

    bool first = true;

    while(!stack.empty()){
        Frame& current = stack.back();

        int i = current.i;
        int j = current.j;

        if(i > j){
            // Caso base: intervalo vazio
            stack.pop_back();
            continue;
        }

        int k = ks[idx(i, j)];

        if(current.visited){
            // Passo 2: Os filhos (esquerda e direita) já foram resolvidos.
            // Agora podemos retirar (imprimir) o k, pois ele é o último.
            if (!first) std::cout << " ";
            std::cout << k;
            first = false;
            
            stack.pop_back(); // Removemos este nó da pilha
        } else {

            // Passo 1: Expandir os filhos (esquerda e direita)
            current.visited = true; // Marcamos que já visitamos este nó

            // Empurrar o filho direito primeiro (para ser processado depois)
            if (k < j) {
                stack.push_back({k + 1, j, false});
            }
            // Empurrar o filho esquerdo
            if (i < k ) {
                stack.push_back({i, k - 1, false});

            }
            
        }
          
    }

    std::cout << "\n";
    
}



int main() {
    // Make std as fast as printf/sscanf
    std::ios_base::sync_with_stdio(0);
    std::cin.tie(0);

    int map[128];

    // map to convert the types to indexs
    map['P'] = 0;
    map['N'] = 1;
    map['A'] = 2;
    map['B'] = 3;
    map['T'] = 4;

    // Reading

    int n;

    // Get quantity
    if (!(std::cin >> n)) return 0;

    int dim = n+2;

    // Pre-allocate memory
    // Evita que o vetor fique se redimensionando e copiando dados.
    // + 2 for Ts
    std::vector<int> potentials(dim);
    std::vector<int> types(dim);

    potentials[0] = 1;
    potentials[n+1] = 1;
    types[0] = map['T'];     // Têm de ter tipo T
    types[n + 1] = map['T']; // Têm de ter tipo T

    // Read input potentials
    for (int i = 0; i < n; i++) {
        std::cin >> potentials[i + 1];

    }

    std::string chain;
    std::cin >> chain;

    // Convert the types
    for (int i = 0; i < n; i++) {
        unsigned char letter = chain[i];
        types[i+1] = map[letter];
    }



    // DP matrix
    // All initialized with 0s
    std::vector<uint64_t> dp(dim*dim, 0);


    // K matrix

    std::vector<int> ks(dim * dim, 0);

    // Lambda que captura 'dim' por valor e faz o cálculo
    auto idx = [dim](int i, int j) { return i * dim + j; };

    // Initialize the diagonal with size = 1 (not counting the Ts)
    for (int i = 1; i <=  n; i++) {
        dp[idx(i, i)] = calculate_energy(potentials[i-1], types[i-1], 
                                         potentials[i], types[i],
                                        potentials[i+1], types[i+1]);

        ks[idx(i, i)] = i; // O k é ele próprio
    }
    // Start with size = 2
    for (int len = 2; len <= n; len++) {
        for (int i = 1; i <= n - len + 1; i++ ) {
            int j = i + len -1;

            uint64_t max_energy = 0;
            int best_k = -1;

            for ( int k = i; k <= j; k++) {
                
                uint64_t cost_left = (k == i) ? 0 : dp[idx(i, k - 1)];
                uint64_t cost_right = (k == j) ? 0 : dp[idx(k + 1, j)];

                uint64_t k_energy = calculate_energy(
                                        potentials[i-1], types[i-1], 
                                        potentials[k], types[k],
                                        potentials[j+1], types[j+1]);


            uint64_t total = cost_left + cost_right + k_energy;
           /* std::cout << "K: " << k << "\n";
            std::cout << "COST LEFT: " << cost_left << "\n";
            std::cout << "COST_ RIGHT: " << cost_right << "\n";
            std::cout << "ACTUAL ENERGY: " << k_energy << "\n";
            std::cout << "TOTAL: " <<  total << "\n";*/
            
            if (total >= max_energy) {
                max_energy = total;
                best_k = k;
            }

            }
            dp[idx(i,j)] = max_energy;
            ks[idx(i,j)] = best_k;
        }
    }

    std::cout << dp[idx(1, n)] << "\n";

    optimal_path(n, dim, ks, idx);

    return 0;
}


