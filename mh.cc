// --- Autors: Adrián Cerezuela i Ramon Ventura ---

#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>
using namespace std;

// --- Declaració, lectura i emmagatzematge de les dades d'entrada. ------------

struct Classe {
  int id;
  int cotxes;
  int num_millores;
  vector<bool> millores;
};

struct Solucio {
  vector<int> ordrecotxes;
  int penalitzacio;
};

int C, M, K;
vector<int> ce, ne;
vector<Classe> classes;
vector<Classe> classes_aux;

string fitxer_entrada, fitxer_sortida;
double inici, final;
double now() { return clock() / double(CLOCKS_PER_SEC); }

void llegir_input() {
  ifstream in(fitxer_entrada);
  in >> C >> M >> K;

  ce.resize(M);
  ne.resize(M);
  classes.resize(K);
  classes_aux.resize(K);

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
int current_pen;
vector<int> current_ordre;
Solucio optima;

void escriure_sortida(const Solucio &c) {
  ofstream out(fitxer_sortida);
  out.setf(ios::fixed);
  out.precision(1);

  final = now();
  double temps_solucio = final - inici;

  out << c.penalitzacio << " " << temps_solucio << endl;

  for (int i = 0; i < int(c.ordrecotxes.size()); ++i) {
    out << c.ordrecotxes[i] << " ";
  }
  out << endl;

  out.close();
}

//------------------------------------------------------------------------------

// --- Algorisme de metaheurística i funcions auxiliars.----------------------

/* Funció auxiliar d'ordenació que prioritza les classes amb més cotxes i, en
cas d'empat, les classes amb major nombre de millores */
bool classe_sorter(const Classe &c1, const Classe &c2) {
  if (c1.cotxes != c2.cotxes)
    return c1.cotxes > c2.cotxes;
  return c1.num_millores > c2.num_millores;
}

/* Troba la següent classe amb el major nombre de cotxes, en cas d'empat
decideix a partir del seu nombre de millores */
int id_seguent_classe() {
  int max_c = 0;
  int classe_esc = -1;
  for (int i = 0; i < K; ++i) {
    if (classes_aux[i].cotxes > 0) {
      if (classes_aux[i].cotxes > max_c) {
        classe_esc = i;
        max_c = classes_aux[i].cotxes;
      } else if (classes_aux[i].cotxes == max_c) {
        if (classes_aux[i].num_millores <
            classes_aux[current_ordre.back()].num_millores) {
          classe_esc = i;
          max_c = classes_aux[i].cotxes;
        }
      }
    }
  }
  return classe_esc;
}

/* Donada una classe i una millora, retorna true si la classe requereix la
millora */
bool requereix_millora(int classe, int millora) {
  return classes[classe].millores[millora] == true;
}

/* Donat un comptador de cotxes que requereixen la millora donada, en una
finestra concreta, actualitza la subpenalitzacio calculada fins al moment */
int actualitza_subpenalitzacio(int &subpenalitzacio, int comptador,
                               int millora) {
  if (comptador > ce[millora]) {
    subpenalitzacio += comptador - ce[millora];
  }
  return subpenalitzacio;
}

/* Funció que, donat un ordre de cotxes, calcula la possbile penalització que
comporta el fet de col·locar l'últim cotxe al vector ordre_cotxes */
int calcula_pen() {
  int subpenalitzacio = 0;
  for (int i = 0; i < M; ++i) {

    if (not requereix_millora(current_ordre.back(), i))
      continue;

    int comptador = 0;
    /* (1) Tram inicial: Si el cotxe està entre 0 i ne-2 -> calcula finestra
      incompleta */
    if (int(current_ordre.size()) < ne[i]) {
      for (int j = 0; j < int(current_ordre.size()); ++j) {
        if (requereix_millora(current_ordre[j], i)) {
          ++comptador;
        }
      }
      subpenalitzacio =
          actualitza_subpenalitzacio(subpenalitzacio, comptador, i);
    } else {
      //(2) Si el cotxe està entre ne-1 i C-ne+1 -> calcula finestra completa
      for (int j = int(current_ordre.size()) - ne[i];
           j < int(current_ordre.size()); ++j) {
        if (requereix_millora(current_ordre[j], i)) {
          ++comptador;
        }
      }
      subpenalitzacio =
          actualitza_subpenalitzacio(subpenalitzacio, comptador, i);
    }

    /* Independentment de si la finestra cap o no,
    (3) Tram final: Si el cotxe està entre C-ne+2 i C -> calcula finestra
    incompleta */
    if (int(current_ordre.size()) == C) {
      for (int j = C - ne[i] + 1; j < C; ++j) {
        comptador = 0;
        for (int k = j; k < C; ++k) {
          if (requereix_millora(current_ordre[k], i)) {
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

// Algorisme greedy per generar un primer ordre
void genera_ordre() {
  for (int i = 0; i < C; ++i) {
    if (i == 0) {
      /* Iteració 0: S'ordenen les classes i es col·loca la que té
            major nombre de cotxes */

      vector<Classe> ordenades = classes_aux;
      sort(ordenades.begin(), ordenades.end(), classe_sorter);
      int primer_id = ordenades[0].id;
      current_ordre.push_back(primer_id);
      --classes_aux[primer_id].cotxes;
    } else {
      /* S'escull, iterant en l'ordre de classes donat, la següent
            classe que segueix el criteri establert */

      int next_id = id_seguent_classe();
      current_ordre.push_back(next_id);
      --classes_aux[next_id].cotxes;
    }
    current_pen += calcula_pen();
  }
}

/* Funció que donat un ordre de cotxes, genera un de nou amb dos cotxes
intercanviats de posició */
vector<int> intercanvi_aleatori(const vector<int> &ordre) {
  int p1 = -1;
  int p2 = -1;
  vector<int> ordre_vei = ordre;

  while (p1 == p2) {
    p1 = rand() % ordre.size() - 1;
    p2 = rand() % ordre.size() - 1;
  }

  swap(ordre_vei[p1], ordre_vei[p2]);
  return ordre_vei;
}

/* Funció auxiliar que troba la penalització del nou ordre de cotxes (amb dues
posicions intercanviades) */
int nova_pen(const vector<int> &ordre) {
  int nova = 0;
  vector<int> ordre_aux;
  for (int i = 0; i < int(ordre.size()); ++i) {
    ordre_aux.push_back(ordre[i]);
    nova += calcula_pen();
  }
  return nova;
}

// Donada una solució, retorna un cert veí de la mateixa
Solucio troba_vei(const Solucio &current) {
  Solucio vei;
  vei.ordrecotxes = intercanvi_aleatori(current.ordrecotxes);
  vei.penalitzacio = nova_pen(vei.ordrecotxes);
  return vei;
}

/* Retorna una certa probabilitat calculada amb la distribució de Boltzmann,
basada en les penalitzacions de la solució actual i una veïna, i la temperatura
T */
double prob(double T, const Solucio &current, const Solucio &vei) {
  return exp((current.penalitzacio - vei.penalitzacio) / T);
}

/* Donada una probabilitat p i dues solucions veïnes, actualitza la solució
actual, permetent, amb certa p, un moviment pitjor. */
void actualitza(double p, Solucio &current, const Solucio &vei) {
  // Genera un nombre aleatori entre 0 i 1
  random_device rd;
  default_random_engine eng(rd());
  uniform_real_distribution<double> distr(0, 1);

  double r = distr(eng);
  if (r <= p)
    current = vei;
}

/* Funció que aplica un algorisme "simulated annealing", en el que la
temperatura es reduïda a cada iteració, i les passes que resulten en solucions
de pitjor qualitat de l'actual (amb l'intenció d'escapar d'un mínim local) són
acceptades. */
void sim_ann(double T) {

  Solucio current = {current_ordre, current_pen};
  if (current.penalitzacio < optima.penalitzacio)
    optima = current;
  escriure_sortida(optima);

  int k = 0;
  while (k < 10000) {
    Solucio vei = troba_vei(current);
    if (vei.penalitzacio < current_pen) {
      current = vei;
      escriure_sortida(current);
      if (current.penalitzacio == 0) {
        optima = current;
        k = 100000;
      }
    } else {
      double p = prob(T, current, vei);
      actualitza(p, current, vei);
    }
    T *= 0.95;
    ++k;
  }
}

int main(int argc, char **argv) {
  fitxer_entrada = argv[1];
  fitxer_sortida = argv[2];

  llegir_input();
  inici = now();

  shuffle(begin(classes), end(classes), default_random_engine());

  current_pen = 0;
  classes_aux = classes;

  /* Per no perdre el vector de classes (modificat a cada iteració)
  utilitzem un d'auxiliar */

  genera_ordre();

  optima = {current_ordre, current_pen};

  while (true) {
    sim_ann(0.95);

    shuffle(begin(classes), end(classes), default_random_engine());
    cout << "shuffled" << endl;

    current_pen = 0;
    current_ordre.clear();
    classes_aux.clear();
    classes_aux = classes;
    genera_ordre();
  }
}
