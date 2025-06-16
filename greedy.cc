// --- Autors: Adrián Cerezuela i Ramon Ventura ---

#include <algorithm>
#include <climits>
#include <ctime>
#include <fstream>
#include <iostream>
#include <vector>
using namespace std;

// --- Declaració, lectura i emmagatzematge de les dades d'entrada. ------------
struct Classe {
  int id;
  int cotxes;
  int num_millores;
  vector<bool> millores;
};

string fitxer_entrada, fitxer_sortida;
double inici, final;
double now() { return clock() / double(CLOCKS_PER_SEC); }

int C, M, K;
vector<int> ce, ne;
vector<Classe> classes;

void llegir_input() {
  ifstream in(fitxer_entrada);
  in >> C >> M >> K;

  ce.resize(M);
  ne.resize(M);
  classes.resize(K);

  for (int i = 0; i < M; ++i) {
    in >> ce[i];
  }
  for (int i = 0; i < M; ++i) {
    in >> ne[i];
  }

  for (int i = 0; i < K; ++i) {
    classes[i].num_millores = 0;
    in >> classes[i].id >> classes[i].cotxes;
    classes[i].millores.resize(M);
    for (int j = 0; j < M; ++j) {
      int millora;
      in >> millora;
      if (millora == 1) {
        classes[i].millores[j] = true;
        ++classes[i].num_millores;
      }
    }
  }
  in.close();
}

//------------------------------------------------------------------------------

// --- Declaració, emmagatzematge i escriptura de les dades de sortida.---------
int penalitzacio;
vector<int> ordre_cotxes;

void escriure_sortida() {
  ofstream out(fitxer_sortida);
  out.setf(ios::fixed);
  out.precision(1);

  final = now();
  double temps_solucio = final - inici;

  out << penalitzacio << " " << temps_solucio << endl;

  for (int &cotxe : ordre_cotxes) {
    out << cotxe << " ";
  }
  out << endl;

  out.close();
}
//------------------------------------------------------------------------------

// --- Algorisme greedy i funcions auxiliars.----------------------

/* Donada una classe i una millora, retorna true si la classe requereix la
millora */
bool requereix_millora(int classe, int millora) {
  return classes[classe].millores[millora] == true;
}

/* Funció auxiliar d'ordenació que prioritza les classes amb més cotxes i, en
cas d'empat, les classes amb major nombre de millores */
bool classe_sorter(const Classe &c1, const Classe &c2) {
  if (c1.cotxes != c2.cotxes)
    return c1.cotxes > c2.cotxes;
  return c1.num_millores > c2.num_millores;
}

/* Donat un comptador de cotxes que requereixen la millora donada, en una
finestra concreta, actualitza la subpenalització calculada fins al moment */
int actualitza_subpenalitzacio(int &subpenalitzacio, int comptador,
                               int millora) {
  if (comptador > ce[millora]) {
    subpenalitzacio += comptador - ce[millora];
  }
  return subpenalitzacio;
}

/* Funció que, donat un ordre de cotxes, calcula la penalització que
comporta el fet de col·locar l'últim cotxe al vector ordre_cotxes. */
int calcula_pen() {
  int subpenalitzacio = 0;
  for (int i = 0; i < M; ++i) {
    /* Si la classe del cotxe que volem calcular la penalització
    conté la millora, actualitza penalització */

    if (not requereix_millora(ordre_cotxes.back(), i))
      continue;

    int comptador = 0;
    /* (1) Tram inicial: Si el cotxe està entre 0 i ne-2 -> calcula finestra
    incompleta */
    if (int(ordre_cotxes.size()) < ne[i]) {
      for (int j = 0; j < int(ordre_cotxes.size()); ++j) {
        if (requereix_millora(ordre_cotxes[j], i)) {
          ++comptador;
        }
      }
      subpenalitzacio =
          actualitza_subpenalitzacio(subpenalitzacio, comptador, i);
    } else {
      // (2) Si el cotxe està entre ne-1 i C-ne+1 -> calcula finestra completa
      for (int j = int(ordre_cotxes.size()) - ne[i];
           j < int(ordre_cotxes.size()); ++j) {
        if (requereix_millora(ordre_cotxes[j], i)) {
          ++comptador;
        }
      }
      subpenalitzacio =
          actualitza_subpenalitzacio(subpenalitzacio, comptador, i);
    }

    /* Independentment de si la finestra cap o no,
    (3) Tram final: Si el cotxe està entre C-ne+2 i C -> calcula finestra
    incompleta */
    if (int(ordre_cotxes.size()) == C) {
      for (int j = C - ne[i] + 1; j < C; ++j) {
        comptador = 0;
        for (int k = j; k < C; ++k) {
          if (requereix_millora(ordre_cotxes[k], i)) {
            ++comptador;
          }
        }
        subpenalitzacio =
            actualitza_subpenalitzacio(subpenalitzacio, comptador, i);
      }
    }
  }
  return subpenalitzacio;
}

/* Funció auxiliar que retorna l'identificador de la classe escollida per
ser col·locada a la següent posició del vector solució. */
int id_seguent_classe() {
  int max_c = 0;
  int classe_esc = -1;

  for (int i = 0; i < K; ++i) {
    if (classes[i].cotxes > 0) {

      if (classes[i].cotxes > max_c) {
        classe_esc = i;
        max_c = classes[i].cotxes;
      } else if (classes[i].cotxes == max_c) {
        if (classes[i].num_millores <
            classes[ordre_cotxes.back()].num_millores) {
          /* Menys millores que el ja col·locat a la posició anterior de la
          solució. */
          classe_esc = i;
          max_c = classes[i].cotxes;
        }
      }
    }
  }
  return classe_esc;
}

void ordre_greedy() {
  for (int i = 0; i < C; ++i) {

    if (i == 0) {
      /* Iteració 0: S'ordenen les classes i es col·loca la que té
      major nombre de cotxes */

      vector<Classe> ordenades = classes;
      sort(ordenades.begin(), ordenades.end(), classe_sorter);
      int primer_id = ordenades[0].id;
      ordre_cotxes.push_back(primer_id);
      --classes[primer_id].cotxes;
    } else {
      /* S'escull, iterant en l'ordre de classes donat, la següent
      classe que segueix el criteri establert */

      int next_id = id_seguent_classe();
      ordre_cotxes.push_back(next_id);
      --classes[next_id].cotxes;
    }
    penalitzacio += calcula_pen();
  }
  escriure_sortida();
}

int main(int argc, char **argv) {
  fitxer_entrada = argv[1];
  fitxer_sortida = argv[2];

  llegir_input();
  inici = now();

  penalitzacio = 0;
  ordre_greedy();
}
//------------------------------------------------------------------------------
