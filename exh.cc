// --- Autors: Adrián Cerezuela i Ramon Ventura. ---

#include <cassert>
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
  vector<int> millores;
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
    in >> classes[i].id >> classes[i].cotxes;
    for (int j = 0; j < M; ++j) {
      int millora;
      in >> millora;
      classes[i].millores.push_back(millora);
    }
  }
  in.close();
}
//------------------------------------------------------------------------------

// --- Declaració, emmagatzematge i escriptura de les dades de sortida.---------
int penalitzacio;
int millor_penalitzacio = INT_MAX;
vector<int> ordre_cotxes;
vector<int> millor_ordre_cotxes;

void escriure_sortida() {
  ofstream out(fitxer_sortida);
  out.setf(ios::fixed);
  out.precision(1);

  final = now();
  double temps_solucio = final - inici;

  out << millor_penalitzacio << " " << temps_solucio << endl;

  for (int i = 0; i < int(millor_ordre_cotxes.size()); ++i) {
    out << millor_ordre_cotxes[i] << " ";
  }
  out << endl;

  out.close();
}
//------------------------------------------------------------------------------

// --- Algorisme de cerca exhaustiva i funcions auxiliars.----------------------

/* Donada una classe i una millora, retorna true si la classe requereix la
millora */
bool requereix_millora(int classe, int millora) {
  return classes[classe].millores[millora] == 1;
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

/* Funció que, donat un ordre de cotxes, calcula la possbile penalització que
comporta el fet de col·locar l'últim cotxe al vector ordre_cotxes. */
int actualitza_penalitzacio() {
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

void genera_ordre(int k) {
  if (k == C and penalitzacio < millor_penalitzacio) {
    millor_penalitzacio = penalitzacio;
    millor_ordre_cotxes = ordre_cotxes;
    escriure_sortida();
  } else if (penalitzacio < millor_penalitzacio) {
    /* Si queden cotxes de la classe, es genera a partir de col·locar un cotxe
    de la mateixa i es desfà el pas posteriorment */
    for (int i = 0; i < K; ++i) {
      if (classes[i].cotxes > 0) {
        ordre_cotxes.push_back(i);
        --classes[i].cotxes;
        int old_pen = penalitzacio;
        penalitzacio += actualitza_penalitzacio();
        genera_ordre(k + 1);
        ordre_cotxes.pop_back();
        ++classes[i].cotxes;
        penalitzacio = old_pen;
      }
    }
  }
}

int main(int argc, char **argv) {
  fitxer_entrada = argv[1];
  fitxer_sortida = argv[2];

  llegir_input();

  penalitzacio = 0;
  inici = now();
  genera_ordre(0);
}
//------------------------------------------------------------------------------
