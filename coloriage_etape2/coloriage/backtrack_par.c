/*
 * backtrack_par.c - Backtracking exact avec des tâches OpenMP.
 *
 * La racine lance les branches des couleurs du premier sommet. Chaque tâche
 * possède une copie indépendante du tableau de couleurs. La recherche est limitée
 * à une profondeur choisie pour éviter de créer un nombre excessif de tâches.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#include "graphe.h"
#include "mesures.h"

static const Graphe *graphe_global;
static int *solution_globale;
static int solution_trouvee;
static int profondeur_tache;
static long long noeuds;
static omp_lock_t verrou_solution;

static int couleur_possible(const Graphe *g, const int *couleurs, int v, int c)
{
    for (int u = 0; u < v; u++)
        if (adjacent(g, v, u) && couleurs[u] == c)
            return 0;
    return 1;
}

static int solution_est_trouvee(void)
{
    int trouvee;
    #pragma omp atomic read
    trouvee = solution_trouvee;
    return trouvee;
}

static void definir_solution_trouvee(void)
{
    #pragma omp atomic write
    solution_trouvee = 1;
}

static int rechercher(const Graphe *g, int *couleurs, int v, int K)
{
    if (solution_est_trouvee())
        return 1;

    if (v == g->n) {
        omp_set_lock(&verrou_solution);
        if (!solution_est_trouvee()) {
            memcpy(solution_globale, couleurs, (size_t)g->n * sizeof *couleurs);
            definir_solution_trouvee();
        }
        omp_unset_lock(&verrou_solution);
        return 1;
    }

    #pragma omp atomic
    noeuds++;

    if (v < profondeur_tache) {
        for (int c = 0; c < K; c++) {
            if (solution_est_trouvee()) break;
            if (!couleur_possible(g, couleurs, v, c)) continue;

            int *branche = coloration_copier(couleurs, g->n);
            branche[v] = c;

            #pragma omp task firstprivate(branche, v, K) shared(g, solution_globale, solution_trouvee)
            {
                rechercher(g, branche, v + 1, K);
                free(branche);
            }
        }
        #pragma omp taskwait
    } else {
        for (int c = 0; c < K; c++) {
            if (solution_est_trouvee()) break;
            if (!couleur_possible(g, couleurs, v, c)) continue;
            couleurs[v] = c;
            if (rechercher(g, couleurs, v + 1, K))
                return 1;
            couleurs[v] = NON_COLORE;
        }
    }
    return solution_est_trouvee();
}

static int est_k_coloriable(const Graphe *g, int K)
{
    #pragma omp atomic write
    solution_trouvee = 0;
    noeuds = 0;
    omp_set_num_threads(omp_get_max_threads());

    #pragma omp parallel shared(g)
    {
        #pragma omp single
        {
            int *racine = coloration_creer(g->n);
            rechercher(g, racine, 0, K);
            free(racine);
        }
    }
    return solution_est_trouvee();
}

static void usage(const char *programme)
{
    fprintf(stderr,
            "Usage:\n"
            "  %s <fichier.col> [options]\n"
            "  %s -g <n> <densite> <graine> [options]\n"
            "Options:\n"
            "  -t <threads>  nombre de threads\n"
            "  -d <profondeur> profondeur maximale de creation des tâches\n"
            "  -r <nb>       nombre de repetitions\n"
            "  -csv <fichier> fichier CSV de mesures\n"
            "  -nom <nom>    nom du graphe dans le CSV\n",
            programme, programme);
    exit(EXIT_FAILURE);
}

static void nom_depuis_fichier(const char *chemin, char *nom, size_t taille)
{
    const char *debut = chemin;
    for (const char *p = chemin; *p; p++)
        if (*p == '/' || *p == '\\') debut = p + 1;
    snprintf(nom, taille, "%s", debut);
    char *point = strrchr(nom, '.');
    if (point != NULL) *point = '\0';
}

int main(int argc, char **argv)
{
    console_utf8();
    Graphe *g = NULL;
    char nom_graphe[64] = "";
    const char *fichier_csv = NULL;
    int repetitions = 1;
    int threads = nb_threads_max();
    profondeur_tache = 3;
    int i;

    if (argc >= 5 && strcmp(argv[1], "-g") == 0) {
        int n = atoi(argv[2]);
        double densite = atof(argv[3]);
        unsigned long long graine = strtoull(argv[4], NULL, 10);
        g = graphe_aleatoire(n, densite, graine);
        snprintf(nom_graphe, sizeof nom_graphe, "alea_%d_%.2f_%llu",
                 n, densite, graine);
        i = 5;
    } else if (argc >= 2 && argv[1][0] != '-') {
        g = graphe_charger(argv[1]);
        nom_depuis_fichier(argv[1], nom_graphe, sizeof nom_graphe);
        i = 2;
    } else {
        usage(argv[0]);
    }
    if (g == NULL) return EXIT_FAILURE;

    for (; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 && i + 1 < argc)
            threads = atoi(argv[++i]);
        else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc)
            profondeur_tache = atoi(argv[++i]);
        else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc)
            repetitions = atoi(argv[++i]);
        else if (strcmp(argv[i], "-csv") == 0 && i + 1 < argc)
            fichier_csv = argv[++i];
        else if (strcmp(argv[i], "-nom") == 0 && i + 1 < argc)
            snprintf(nom_graphe, sizeof nom_graphe, "%s", argv[++i]);
        else {
            fprintf(stderr, "Option inconnue ou incomplete: %s\n", argv[i]);
            usage(argv[0]);
        }
    }
    if (threads < 1) threads = 1;
    if (repetitions < 1) repetitions = 1;
    if (profondeur_tache < 0) profondeur_tache = 0;

    omp_set_num_threads(threads);
    omp_init_lock(&verrou_solution);
    solution_globale = coloration_creer(g->n);
    graphe_global = g;

    printf("=== Backtracking OpenMP : %s (%d threads) ===\n",
           nom_graphe, threads);
    graphe_afficher(g);

    int chi = 0;
    double somme = 0.0;
    for (int r = 1; r <= repetitions; r++) {
        double debut = chrono();
        chi = 0;
        for (int K = 1; K <= g->n; K++) {
            #pragma omp atomic write
            solution_trouvee = 0;
            if (est_k_coloriable(g, K)) {
                chi = K;
                break;
            }
        }
        double temps = chrono() - debut;
        somme += temps;

        if (!coloration_est_valide(g, solution_globale)) {
            fprintf(stderr, "Erreur : aucune solution valide trouvee\n");
            omp_destroy_lock(&verrou_solution);
            free(solution_globale);
            graphe_liberer(g);
            return EXIT_FAILURE;
        }
        if (repetitions > 1) printf("  Repetition %d : %.6f s\n", r, temps);
        if (fichier_csv != NULL)
            mesure_ecrire(fichier_csv, "backtrack", "par", nom_graphe,
                          g->n, g->m, threads, r, temps, chi);
    }

    printf("Nombre chromatique : chi(G) = %d\n", chi);
    printf("Coloration optimale :\n");
    coloration_afficher(g, solution_globale);
    printf("Temps %s : %.6f s\n",
           repetitions > 1 ? "moyen" : "total", somme / repetitions);
    if (fichier_csv != NULL) printf("Mesures ajoutees dans %s\n", fichier_csv);

    omp_destroy_lock(&verrou_solution);
    free(solution_globale);
    graphe_liberer(g);
    return EXIT_SUCCESS;
}
