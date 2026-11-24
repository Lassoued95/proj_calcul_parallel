/*
 * glouton_seq.c - Coloriage glouton sequentiel
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graphe.h"
#include "mesures.h"

static int couleur_possible(const Graphe *g, const int *couleurs, int v, int c)
{
    for (int u = 0; u < g->n; u++)
        if (adjacent(g, v, u) && couleurs[u] == c)
            return 0;
    return 1;
}

static int glouton(const Graphe *g, int *couleurs)
{
    for (int v = 0; v < g->n; v++)
        couleurs[v] = NON_COLORE;

    for (int v = 0; v < g->n; v++) {
        int c = 0;
        while (!couleur_possible(g, couleurs, v, c))
            c++;
        couleurs[v] = c;
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
        if (strcmp(argv[i], "-r") == 0 && i + 1 < argc)
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
    if (repetitions < 1) repetitions = 1;

    printf("=== Coloriage glouton sequentiel : %s ===\n", nom_graphe);
    graphe_afficher(g);
    int *couleurs = coloration_creer(g->n);
    int nb_couleurs = 0;
    double somme = 0.0;

    for (int r = 1; r <= repetitions; r++) {
        double debut = chrono();
        nb_couleurs = glouton(g, couleurs);
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
            mesure_ecrire(fichier_csv, "glouton", "seq", nom_graphe,
                          g->n, g->m, 1, r, temps, nb_couleurs);
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
