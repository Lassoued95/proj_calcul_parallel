/*
 * glouton_par.c - Version OpenMP du coloriage glouton.
 *
 * La construction reste sequentielle. Pour chaque sommet, l'examen des voisins
 * est parallelise: chaque thread parcourt une partie de la ligne de la matrice.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#include "graphe.h"
#include "mesures.h"

static int glouton_parallel(const Graphe *g, int *couleurs, int threads)
{
    for (int v = 0; v < g->n; v++)
        couleurs[v] = NON_COLORE;

    omp_set_num_threads(threads);
    for (int v = 0; v < g->n; v++) {
        unsigned char *interdit = calloc((size_t)g->n, sizeof *interdit);
        if (interdit == NULL) {
            fprintf(stderr, "Erreur : memoire insuffisante\n");
            exit(EXIT_FAILURE);
        }

        #pragma omp parallel for schedule(static)
        for (int u = 0; u < g->n; u++) {
            if (adjacent(g, v, u) && couleurs[u] != NON_COLORE) {
                #pragma omp atomic write
                interdit[couleurs[u]] = 1;
            }
        }

        int c = 0;
        while (c < g->n && interdit[c])
            c++;
        couleurs[v] = c;
        free(interdit);
    }
    return coloration_nb_couleurs(couleurs, g->n);
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

static void usage(const char *programme)
{
    fprintf(stderr,
            "Usage:\n"
            "  %s <fichier.col> [options]\n"
            "  %s -g <n> <densite> <graine> [options]\n"
            "Options:\n"
            "  -t <threads>    nombre de threads\n"
            "  -r <nb>         nombre de repetitions\n"
            "  -csv <fichier>  fichier CSV de mesures\n"
            "  -nom <nom>      nom du graphe dans le CSV\n",
            programme, programme);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    console_utf8();
    Graphe *g = NULL;
    char nom_graphe[64] = "";
    const char *fichier_csv = NULL;
    int repetitions = 1;
    int threads = nb_threads_max();
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

    printf("=== Coloriage glouton OpenMP : %s (%d threads) ===\n",
           nom_graphe, threads);
    graphe_afficher(g);
    int *couleurs = coloration_creer(g->n);
    int nb_couleurs = 0;
    double somme = 0.0;

    for (int r = 1; r <= repetitions; r++) {
        double debut = chrono();
        nb_couleurs = glouton_parallel(g, couleurs, threads);
        double temps = chrono() - debut;
        somme += temps;
        if (!coloration_est_valide(g, couleurs)) {
            fprintf(stderr, "Erreur : la coloration gloutonne est invalide\n");
            free(couleurs);
            graphe_liberer(g);
            return EXIT_FAILURE;
        }
        if (repetitions > 1) printf("  Repetition %d : %.6f s\n", r, temps);
        if (fichier_csv != NULL)
            mesure_ecrire(fichier_csv, "glouton", "par", nom_graphe,
                          g->n, g->m, threads, r, temps, nb_couleurs);
    }

    printf("Nombre de couleurs : %d\n", nb_couleurs);
    coloration_afficher(g, couleurs);
    printf("Temps %s : %.6f s\n",
           repetitions > 1 ? "moyen" : "total", somme / repetitions);
    if (fichier_csv != NULL) printf("Mesures ajoutees dans %s\n", fichier_csv);

    free(couleurs);
    graphe_liberer(g);
    return EXIT_SUCCESS;
}
