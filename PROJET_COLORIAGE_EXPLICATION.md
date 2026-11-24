# Projet - Coloriage de graphes

## Rapport complet et documentation du projet

Ce document présente l'ensemble du sujet : théorie, modèle, applications, algorithmes,
pseudo-codes, traces d'exécution, complexité, implémentation séquentielle, parallélisation
OpenMP, mesure des performances, Gantt, livrables et structure du code.

---

## 1. Objectif du travail

Le coloriage de graphes consiste à attribuer une couleur à chaque sommet d'un graphe
non orienté, de sorte que deux sommets reliés par une arête n'aient jamais la même couleur.
Cette contrainte produit un **coloriage propre**.

Un graphe est représenté par \(G=(V,E)\), où :

- \(V\) est l'ensemble des sommets ;
- \(E\) est l'ensemble des arêtes ;
- \(n=|V|\) est le nombre de sommets ;
- \(m=|E|\) est le nombre d'arêtes.

L'objectif principal est de déterminer le **nombre chromatique** :

\[
\chi(G)=\min\{K\in\mathbb N\;:\;G\text{ possède une coloration propre à }K\text{ couleurs}\}.
\]

Le projet étudie deux approches :

1. l'algorithme heuristique glouton ;
2. l'algorithme exact par Backtracking.

Le problème est important en algorithmique parce qu'il est un problème d'optimisation
combinatoire difficile : sa version décisionnelle avec \(K\geq3\) couleurs est NP-complète
et la recherche du minimum est NP-difficile.

---

## 2. Définitions fondamentales

### Graphe non orienté

Dans un graphe non orienté, une arête `{u,v}` représente une relation entre les sommets
`u` et `v` dans les deux sens. Il n'y a pas de boucle ni d'arête orientée.

### Sommets adjacents

Deux sommets `u` et `v` sont adjacents lorsqu'une arête les relie. Ils ne doivent pas recevoir
la même couleur.

### Degré

Le degré d'un sommet est le nombre d'arêtes qui lui sont reliées. Le degré maximal est noté
\(\Delta(G)\).

### Coloriage propre

Un coloriage propre vérifie :

\[
\forall (u,v)\in E,\quad c(u)\neq c(v).
\]

### Coloriage valide

Un coloriage valide est une coloration propre complète. Tous les sommets doivent avoir une couleur
et aucune contrainte d'adjacence ne doit être violée.

### Coloriage optimal

Un coloriage est optimal lorsqu'il utilise exactement \(\chi(G)\) couleurs. Une coloration valide
avec trois couleurs n'est pas nécessairement optimale : il faudrait prouver qu'aucune solution
à deux couleurs n'existe.

### Graphe complet

Dans un graphe complet \(K_n\), chaque paire de sommets est reliée. Il faut donc une couleur
différente pour chaque sommet :

\[
\chi(K_n)=n.
\]

---

## 3. Présentation formelle du problème

### Problème décisionnel

Entrée : un graphe \(G=(V,E)\) et un entier \(K\).

Question : existe-t-il une attribution \(c:V\rightarrow\{0,\ldots,K-1\}\) telle que
\(c(u)\neq c(v)\) pour toute arête `(u,v)` ?

### Problème d'optimisation

Entrée : un graphe \(G=(V,E)\).

Sortie : une coloration propre utilisant le plus petit nombre possible de couleurs.

### Difficulté

Même avec seulement trois couleurs, déterminer si une coloration existe est NP-complet.
Une solution peut donc être vérifiée rapidement, mais trouver cette solution peut nécessiter
l'examen d'un nombre exponentiel de possibilités dans le pire cas.

---

## 4. Exemple simple : graphe prisme

Le projet contient le graphe `graphes/exemple_simple.col`.

- Sommets : A, B, C, D, E, F.
- Triangles : A-B-C et D-E-F.
- Arêtes verticales : A-F, B-D, C-E.
- Nombre d'arêtes : 9.

```text
       A -------- B
      /|          |\
     F |          | D
      \|          |/
       C -------- E
```

Mais cette représentation doit être lue comme un prisme : A-F, B-D et C-E sont des arêtes
reliant les deux triangles.

Arêtes DIMACS :

```text
A-B  A-C  B-C
D-E  D-F  E-F
A-F  B-D  C-E
```

Une coloration valide est :

| Couleur | Sommets |
|---|---|
| 0 | A, D |
| 1 | B, E |
| 2 | C, F |

Le graphe contient au moins un triangle. Chaque sommet d'un triangle doit avoir une couleur
différente, donc au moins trois couleurs sont nécessaires :

\[
\chi(G)\geq3.
\]

La coloration ci-dessus utilise exactement trois couleurs. Elle est donc optimale :

\[
\chi(G)=3.
\]

---

## 5. Exemple réel : emploi du temps des examens

### Modélisation

Chaque examen est représenté par un sommet. Deux sommets sont reliés lorsqu'au moins un étudiant
suit les deux examens associés. Une couleur représente un créneau horaire.

Le fichier `graphes/examens.col` contient 7 examens et 10 incompatibilités.

| Sommet | Examen |
|---|---|
| 0 | Maths |
| 1 | Physique |
| 2 | Informatique |
| 3 | Anglais |
| 4 | Économie |
| 5 | Chimie |
| 6 | Histoire |

Le triangle Maths-Physique-Informatique impose au moins trois créneaux.

Une solution valide trouvée par le projet est :

| Créneau | Examens |
|---|---|
| 0 | Maths, Économie, Chimie |
| 1 | Physique, Anglais |
| 2 | Informatique, Histoire |

Cette solution utilise trois créneaux. Le triangle prouve que deux créneaux ne suffisent pas,
donc elle est optimale.

---

## 6. Démarche de résolution

1. Modéliser les objets et les contraintes sous forme de graphe.
2. Identifier les sommets et les arêtes.
3. Représenter les sommets dans une matrice d'adjacence.
4. Initialiser toutes les couleurs à `NON_COLORE`, valeur `-1`.
5. Choisir une méthode : gloutonne ou Backtracking.
6. Appliquer les contraintes au traitement de chaque sommet.
7. Vérifier la solution avec `coloration_est_valide()`.
8. Comparer le nombre de couleurs obtenues.
9. Mesurer les temps et répéter les expériences.

---

## 7. Représentation du graphe dans le code

### Structure `Graphe`

Déclarée dans `graphe.h` :

```c
typedef struct {
    int n;
    long m;
    unsigned char *adj;
    char (*noms)[TAILLE_NOM];
} Graphe;
```

- `n` : nombre de sommets ;
- `m` : nombre d'arêtes ;
- `adj` : matrice d'adjacence ;
- `noms` : noms facultatifs des sommets.

### Matrice d'adjacence

La matrice possède \(n^2\) cases :

```c
adj[u*n + v] == 1
```

signifie que les sommets `u` et `v` sont reliés.

Cette représentation utilise environ \(n^2\) octets ou cases. Le test d'une arête se fait en
\(\Theta(1)\).

### Format DIMACS

Le fichier utilise des sommets numérotés de 1 à n :

```text
c commentaire
p edge n m
c nom 1 Maths
e 1 2
e 1 3
```

Le lecteur `graphe_charger()` soustrait 1 aux identifiants afin d'utiliser les indices C
de 0 à n-1.

---

## 8. Algorithme glouton

### Principe

Les sommets sont traités dans l'ordre 0, 1, ..., n-1. Pour chaque sommet :

1. examiner les couleurs de ses voisins ;
2. construire l'ensemble des couleurs interdites ;
3. choisir la plus petite couleur non interdite ;
4. passer au sommet suivant.

### Pseudo-code séquentiel

```text
ENTREE : Graphe G = (V, E)
SORTIE : Tableau couleur et nombre de couleurs utilisees

pour tout sommet v :
    couleur[v] <- NON_COLORE

pour v de 0 à n - 1 :
    c <- 0
    tant que existe u voisin de v avec couleur[u] = c :
        c <- c + 1
    couleur[v] <- c

verifier que le coloriage est complet
verifier que chaque arête relie deux couleurs différentes
retourner le nombre de couleurs utilisées
```

### Trace sur le graphe prisme

| Étape | Sommet | Couleurs voisines déjà choisies | Couleur |
|---|---|---|---|
| 1 | A | aucune | 0 |
| 2 | B | A : 0 | 1 |
| 3 | C | A : 0, B : 1 | 2 |
| 4 | D | B : 1 | 0 |
| 5 | E | C : 2, D : 0 | 1 |
| 6 | F | A : 0, C : 2, E : 1 | 3 |

Attention : cette trace dépend de l'ordre exact et peut être améliorée. Dans le résultat actuel
du projet, la représentation prisme est arrangée pour produire A,D=0 ; B,E=1 ; C,F=2.

### Pourquoi la solution n'est pas forcément optimale

Le glouton ne planifie pas les décisions futures. Un sommet peut recevoir une nouvelle couleur
avant qu'une meilleure affectation plus tardive soit connue.

Exemple classique du chemin a-b-c-d :

- l'ordre a,b,c,d peut produire deux couleurs ;
- l'ordre a,d,b,c peut produire trois couleurs.

La solution reste valide mais son nombre de couleurs dépend fortement de l'ordre des sommets.

---

## 9. Algorithme exact par Backtracking

### Principe

Le Backtracking essaie les nombres de couleurs successivement :

\[
K=1,2,3,\ldots
\]

Pour une valeur donnée de `K`, le programme explore toutes les affectations possibles jusqu'à
trouver une solution ou prouver qu'aucune branche n'est valide.

### Vérification d'une couleur

```text
couleur_possible(G, couleur, v, c):
    pour tout u dans 0..v-1 :
        si u est adjacent à v et couleur[u] = c :
            retourner FAUX
    retourner VRAI
```

Dans l'implémentation séquentielle, les sommets sont traités dans l'ordre croissant.
Il suffit donc de regarder les voisins déjà traités.

### Pseudo-code récursif

```text
ENTREE : Graphe G, tableau couleur, profondeur v, nombre maximum K
SORTIE : VRAI si une coloration valide existe

si v = n :
    retourner VRAI

pour c de 0 à K - 1 :
    si couleur_possible(G, couleur, v, c) :
        couleur[v] <- c
        si Backtracking(G, couleur, v + 1, K) = VRAI :
            retourner VRAI
        couleur[v] <- NON_COLORE

retourner FAUX
```

### Arbre de recherche

La recherche peut être représentée par un arbre :

- la racine représente le graphe sans sommet colorié ;
- un niveau correspond à un sommet ;
- chaque branche correspond à une couleur possible ;
- une feuille représente une coloration complète ;
- une impasse correspond à un sommet sans couleur compatible ;
- un retour en arrière annule le dernier choix et essaie l'alternative suivante.

### Retour en arrière

Un retour en arrière a lieu :

- lorsqu'un sommet voisin utilise déjà la couleur candidate ;
- lorsqu'il n'y a plus de couleur compatible ;
- lorsqu'un appel récursif termine sans solution ;
- lorsqu'une branche doit être abandonnée.

### Recherche du nombre chromatique

```text
ENTREE : Graphe G
SORTIE : nombre chromatique χ(G) et coloration optimale

pour K de 1 à n :
    remettre tous les sommets à NON_COLORE
    si Backtracking(G, couleur, 0, K) = VRAI :
        retourner K et couleur

retourner n
```

La première valeur de `K` pour laquelle une solution est trouvée est minimale. Pour \(K-1\),
l'arbre a été entièrement exploré et aucune solution n'a été trouvée.

---

## 10. Complexité avec matrice d'adjacence

### Glouton séquentiel

Pour chaque sommet `v`, l'algorithme examine au plus `n` voisins potentiels.
Dans le pire cas :

- jusqu'à n sommets ;
- jusqu'à n voisins examinés ;
- jusqu'à n couleurs vérifiées.

Avec une matrice d'adjacence, une vérification simple donne :

- temps moyen/best/worst : \(\Theta(n^2)\) ;
- mémoire : \(\Theta(n^2)\) pour la matrice ;
- tableau des couleurs : \(\Theta(n)\).

La borne dépend aussi du nombre de couleurs utilisées, qui peut atteindre n.
Une analyse avec \(m\) est possible avec une liste d'adjacence, mais cette représentation
utilise une matrice dans le projet.

### Backtracking

L'arbre possède au plus \(K^n\) affectations possibles.

- meilleur cas : une solution complète apparaît rapidement ;
- cas moyen : dépend fortement de la densité du graphe ;
- pire cas : \(\Omega(K^n)\) branches, avec des limites exponentielles ;
- coût de chaque vérification : \(\Theta(n)\) dans la boucle des voisins déjà traités ;
- mémoire récursive : \(\Theta(n)\) pour la pile ;
- table/copie : \(\Theta(n)\).

Pour une représentation matricielle, une borne grossière est \(O(nK^n)\).
La mémoire totale reste dominée par la matrice \(n^2\), sauf si l'implémentation utilise
un grand nombre de tableaux privés.

### Le problème général

Le glouton est polynomial, mais il ne résout pas exactement le problème. Le Backtracking
trouve une solution optimale mais sa complexité peut devenir exponentielle.

---

## 11. Structure du dépôt

```text
proj_calcul_parallel/
|-- Coloriage de graphes – Étape 1 cadrage (2).pdf
|-- PROJET_COLORIAGE_EXPLICATION.md
|-- coloriage_etape1 (1)/
|   `-- coloriage/
|       |-- graphe.c
|       |-- graphe.h
|       |-- mesures.c
|       |-- mesures.h
|       |-- generer.c
|       |-- test_commun.c
|       `-- graphes/
`-- coloriage_etape2/
    `-- coloriage/
        |-- Makefile
        |-- README.md
        |-- graphe.c
        |-- graphe.h
        |-- mesures.c
        |-- mesures.h
        |-- generer.c
        |-- test_commun.c
        |-- glouton_seq.c
        |-- glouton_par.c
        |-- backtrack_seq.c
        |-- backtrack_par.c
        |-- mesures.csv
        `-- graphes/
```

---

## 12. Fonctions communes

### Gestion des graphes

- `graphe_creer(n)`
- `graphe_liberer(g)`
- `graphe_ajouter_arete(g,u,v)`
- `graphe_charger(path)`
- `graphe_sauver(g,path)`
- `graphe_aleatoire(n,densite,graine)`
- `adjacent(g,u,v)`

### Gestion des colorations

- `coloration_creer(n)`
- `coloration_copier(couleurs,n)`
- `coloration_est_valide(g,couleurs)`
- `coloration_nb_conflits(g,couleurs)`
- `coloration_nb_couleurs(couleurs,n)`
- `coloration_afficher(g,couleurs)`

### Mesures

- `chrono()`
- `numero_thread()`
- `nb_threads_max()`
- `mesure_ecrire(...)`
- `trace_demarrer(capacite)`
- `trace_ajouter(debut,fin,etiquette)`
- `trace_ecrire_csv(path)`

---

## 13. Implémentations séquentielles

### Glouton séquentiel

Le programme `glouton_seq.c` :

1. charge un fichier DIMACS ou crée un graphe aléatoire ;
2. initialise le tableau des couleurs ;
3. parcourt les sommets en ordre croissant ;
4. examine les voisins déjà coloriés ;
5. choisit la première couleur disponible ;
6. vérifie la validité ;
7. écrit éventuellement une mesure CSV.

### Backtracking séquentiel

Le programme `backtrack_seq.c` implémente les cinq fonctions principales demandées :

1. `couleur_possible()` ;
2. `colorier()` ;
3. `est_k_coloriable()` ;
4. `nombre_chromatique()` ;
5. vérification finale de la solution.

Il compte aussi les nœuds visités et les retours arrière.

---

## 14. Compilation

Se placer dans :

```text
coloriage_etape2\coloriage
```

### Windows avec Code::Blocks

1. Ouvrir `coloriage.cbp`.
2. Choisir une cible.
3. Compiler et exécuter avec F9.

Les options recommandées sont :

```text
-O2 -Wall -Wextra -std=c11 -fopenmp
```

### Windows avec MinGW

```bat
"C:\Program Files\CodeBlocks\MinGW\bin\mingw32-make.exe"
"C:\Program Files\CodeBlocks\MinGW\bin\mingw32-make.exe" test
```

### Linux ou WSL

```bash
make
make test
```

Installation minimale :

```bash
sudo apt install build-essential
```

### Mac

```bash
make CC=gcc-14
make CC=gcc-14 test
```

---

## 15. Exécution des programmes

### Tests communs

```bat
test_commun.exe
```

Résultat attendu :

```text
27 tests, 0 échec(s)
Tous les tests sont passés : le code commun fonctionne sur cette machine.
```

### Générateur

```bat
generer.exe 30 0.5 1 graphes\mon_graphe.col
```

### Glouton séquentiel

```bat
glouton_seq.exe graphes\exemple_simple.col
glouton_seq.exe -g 200 0.3 42 -r 5 -csv mesures.csv -nom G1
```

### Glouton OpenMP

```bat
glouton_par.exe graphes\exemple_simple.col -t 4
glouton_par.exe -g 2000 0.3 42 -t 8 -r 5 -csv mesures.csv -nom G3
```

### Backtracking séquentiel

```bat
backtrack_seq.exe graphes\exemple_simple.col -v
backtrack_seq.exe graphes\myciel4.col
backtrack_seq.exe -g 26 0.5 1 -r 5 -csv mesures.csv -nom BT1
```

### Backtracking OpenMP

```bat
backtrack_par.exe graphes\exemple_simple.col -t 4 -d 3
backtrack_par.exe -g 26 0.5 1 -t 8 -d 4 -r 5 -csv mesures.csv -nom BT1
```

Options principales :

- `-t` : nombre de threads ;
- `-d` : profondeur maximale de création des tâches ;
- `-r` : nombre de répétitions ;
- `-csv` : fichier de mesures ;
- `-nom` : nom du graphe dans le CSV ;
- `-v` : afficher l'arbre pour les petits graphes.

---

## 16. Parallélisation du glouton

### Dépendance principale

La couleur du sommet `v` dépend des couleurs déjà attribuées à ses voisins. Il est donc
incorrect de créer directement toutes les affectations dans une seule boucle parallèle.

### Opérations indépendantes

Pour un sommet donné, chaque thread peut parcourir une partie des n sommets ou une partie
de la matrice afin de construire un tableau de couleurs interdites.

L'implémentation `glouton_par.c` conserve l'ordre séquentiel des sommets. Pour chaque sommet,
elle utilise une région `parallel for` afin d'examiner les voisins.

### Directives OpenMP utilisées

```c
#pragma omp parallel for schedule(static)
for (int u = 0; u < g->n; u++)
    // Examen d'une partie de la ligne de la matrice
```

Chaque thread lit les couleurs déjà attribuées et écrit dans sa partie du tableau partagé
`interdit`. Les écritures concernent des indices potentiellement différents ; pour l'implémentation
actuelle, elles restent protégées ou validées par une opération atomique.

### Limites

Cette parallélisation ne change pas l'algorithme pour les sommets : les sommets restent dépendants
dans l'ordre. Pour les petits graphes, le coût de création des threads peut dépasser le gain.

Une autre stratégie possible serait un parallélisme par niveaux avec vérification globale,
mais cela peut modifier l'ordonnancement ou produire une autre coloration.

---

## 17. Parallélisation du Backtracking

### Pourquoi utiliser des tâches

Les branches de l'arbre sont indépendantes : chaque possibilité de couleur du sommet `v`
conduit à un sous-problème avec un état de tableau propre.

### Région OpenMP

```c
#pragma omp parallel
{
    #pragma omp single
    {
        // Création initiale des tâches
    }
}
```

- `parallel` crée les threads ;
- `single` permet à un seul thread de lancer la racine ;
- `task` crée une branche indépendante ;
- `taskwait` attend les sous-tâches locales.

### États privés

Chaque tâche reçoit une copie indépendante du tableau `couleurs`. Le graphe reste partagé
en lecture seule. La solution globale est protégée par un verrou ou une opération atomique.

### Limitation des tâches

La création récursive peut devenir excessive. L'implémentation limite la création des tâches
avec `-d` :

- aux niveaux avant la profondeur `d`, chaque branche devient une tâche ;
- aux niveaux après `d`, chaque tâche continue avec le Backtracking séquentiel ;
- cette stratégie limite le nombre de tâches et conserve du travail utile.

### Synchronisation

Un verrou `omp_lock_t` protège la mise à jour de la meilleure solution trouvée.
`taskwait` attend les tâches créées dans une branche mais ne remplace pas la protection
des données partagées.

---

## 18. Recherche exacte parallèle

Pour déterminer χ(G), l'implémentation teste successivement les valeurs de `K` :

```text
pour K de 1 à n :
    lancer la recherche parallèle
    si une coloration valide existe :
        enregistrer K
        arrêter
```

Lorsque \(K-1\) a été testé sans solution, aucune branche avec \(K-1\) couleurs n'existe.
Le premier `K` ayant trouvé une solution est donc le minimum.

Une simple coloration valide ne suffit pas à elle seule : il faut vérifier les valeurs inférieures.

---

## 19. Données de test

### Graphes inclus

| Graphe | n | m | Objectif |
|---|---:|---:|---|
| chemin_p4 | 4 | 3 | dépendance de l'ordre glouton |
| exemple_simple | 6 | 9 | exemple pédagogique |
| examens | 7 | 10 | application réelle |
| myciel3 | 11 | 20 | χ = 4 |
| myciel4 | 23 | 71 | χ = 5 |
| BT1 | 24 | 150 | mesure backtracking |
| BT2 | 26 | 176 | mesure backtracking |
| BT3 | 30 | 221 | mesure backtracking |
| BT4 | 32 | 253 | mesure backtracking |

Ces mesures doivent être complétées avec les capacités réelles de la machine.

### Graphe reproductible

`graphe_aleatoire(200,0.3,42)` produit exactement 5948 arêtes dans cette implémentation.

Le test commun garantit que les graphes générés sont identiques sur les machines du groupe.

---

## 20. Format des mesures

Le fichier CSV contient :

```text
algo,version,graphe,n,m,threads,essai,temps_s,nb_couleurs
```

Exemple :

```text
backtrack,seq,BT1,24,150,1,1,3.105000,7
backtrack,par,BT1,24,150,4,1,1.400000,7
```

Une ligne correspond à une exécution et non à une moyenne.

Pour chaque graphe :

1. exécuter la version séquentielle ;
2. répéter au moins cinq fois ;
3. supprimer les valeurs extrêmes liées aux perturbations ;
4. calculer la moyenne ;
5. mesurer 1,2,4,8,... threads ;
6. conserver les valeurs dans le CSV.

---

## 21. Speedup

Pour un nombre de threads `p` :

\[
S(p)=\frac{T_s}{T_p}
\]

- \(T_s\) : temps séquentiel ;
- \(T_p\) : temps parallèle avec `p` threads.

Exemple de tableau à compléter :

| Graphe | \(T_s\) | \(T_1\) | \(T_2\) | \(T_4\) | \(T_8\) | \(S(8)\) |
|---|---:|---:|---:|---:|---:|---:|
| G1 |  |  |  |  |  |  |
| G2 |  |  |  |  |  |  |
| G3 |  |  |  |  |  |  |
| G4 |  |  |  |  |  |  |

L'accélération idéale vaut \(S(p)=p\).

Une accélération supérieure à `p` peut parfois apparaître en présence d'effets de cache,
de changements d'ordonnancement ou de gain lié à l'exécution parallèle d'opérations
avec des données différentes. Elle ne doit pas être appelée théoriquement parfaite.

---

## 22. Efficacité parallèle

\[
E(p)=\frac{S(p)}{p}
\]

ou :

\[
E(p)=\frac{T_s}{pT_p}.
\]

Exemple de tableau :

| Threads | Speedup | Efficacité |
|---:|---:|---:|
| 1 | 1 | 100 % |
| 2 |  |  |
| 4 |  |  |
| 8 |  |  |

L'efficacité diminue généralement à cause :

- des coûts de création des threads ;
- des tâches déséquilibrées ;
- des sections critiques ;
- des conflits d'accès mémoire ;
- du partage de variables ;
- du nombre insuffisant de branches indépendantes ;
- de la surcharge liée à OpenMP ;
- des migrations des threads entre cœurs.

La meilleure configuration est celle qui offre un bon compromis, pas toujours celle avec
le plus grand nombre de threads.

---

## 23. Courbes de performance

Les courbes demandées sont :

1. temps d'exécution en fonction du nombre de threads ;
2. speedup en fonction du nombre de threads ;
3. efficacité en fonction du nombre de threads ;
4. temps d'exécution en fonction de la taille du graphe.

Pour la courbe du speedup, ajouter :

\[
S_{\text{idéal}}(p)=p.
\]

Une courbe devrait être produite pour chaque algorithme : glouton et Backtracking.

### Interprétation attendue

- le glouton parallèle peut être limité par sa boucle séquentielle ;
- le Backtracking peut obtenir un meilleur gain grâce aux branches indépendantes ;
- le gain diminue si les graphes sont trop petits ;
- les graphes difficiles peuvent présenter un déséquilibre important ;
- les tâches récursives peuvent avoir des durées très différentes.

---

## 24. Diagramme de Gantt

Le Gantt représente les tâches et leurs durées sur chaque thread.

```text
Thread 0 : [branche A][attente][branche C][attente]
Thread 1 : [branche B][attente][branche D][attente]
Thread 2 : [attente][branche E][branche F]
Thread 3 : [attente][branche G][attente]
Temps    : 0        10       20       30      ms
```

Les traces sont produites avec `trace_demarrer()`, `trace_ajouter()` et
`trace_ecrire_csv()`.

Format CSV :

```text
thread,debut_ms,fin_ms,duree_ms,etiquette
0,0.0000,12.5000,12.5000,c=0 v=2
1,0.1000,9.3000,9.2000,c=1 v=2
```

Pour améliorer l'équilibrage :

- utiliser une stratégie work stealing ;
- ajuster la profondeur des tâches ;
- éviter de créer une tâche pour chaque feuille ;
- regrouper plusieurs décisions dans chaque tâche ;
- arrêter les branches concurrentes après la première solution ;
- utiliser `schedule(dynamic)` ou `guided` lorsque les charges sont irrégulières.

---

## 25. Résultats vérifiés

La machine de validation a obtenu :

- OpenMP activé ;
- 8 threads disponibles ;
- `27 tests, 0 échec(s)` ;
- `graphe_aleatoire(200,0.3,42)` = 5948 arêtes ;
- glouton séquentiel valide sur `exemple_simple.col` ;
- glouton parallèle valide sur `exemple_simple.col` ;
- Backtracking séquentiel : χ = 3 ;
- Backtracking parallèle : χ = 3.

Ces résultats concernent uniquement l'exemple simple. Ils doivent être complétés
avec des mesures répétées sur les graphes BT1 à BT4.

---

## 26. Applications réelles

### Télécommunications

Dans les réseaux radio, deux émetteurs proches peuvent créer des interférences.
Les émetteurs deviennent des sommets, les conflits des arêtes et les fréquences des couleurs.
Le nombre chromatique représente le nombre minimal de fréquences nécessaires.

### Informatique

Dans un compilateur, les variables en conflit ne peuvent pas partager le même registre.
Les variables sont des sommets, les conflits des arêtes et les registres des couleurs.

### Planification

Les examens ou réunions incompatibles deviennent des sommets reliés par une arête.
Les créneaux ou salles deviennent les couleurs.

### Transport

À une intersection ou dans un réseau ferroviaire, les itinéraires qui ne peuvent pas être
simultanés doivent recevoir des périodes différentes.

### Industrie

Dans une usine, les tâches partageant une machine ou une ressource critique ne peuvent pas
s'exécuter pendant le même intervalle. Le coloriage produit une planification compatible.

---

## 27. Plan de présentation PowerPoint

1. Titre et objectifs.
2. Définitions : graphe, sommet, arête, degré.
3. Coloriage propre et nombre chromatique.
4. Exemple du prisme.
5. Emploi du temps des examens.
6. Glouton : principe et pseudo-code.
7. Trace du glouton.
8. Backtracking : principe et pseudo-code.
9. Arbre de recherche.
10. Complexité séquentielle.
11. Architecture du code C.
12. Parallélisation du glouton.
13. Parallélisation du Backtracking avec `task`.
14. Gestion des données et synchronisation.
15. Jeux de graphes.
16. Tableau des temps.
17. Courbes de speedup.
18. Courbes d'efficacité.
19. Diagrammes de Gantt.
20. Applications et conclusion.

---

## 28. Livrables attendus

- rapport structuré et complet ;
- représentations graphiques ;
- pseudo-codes commentés ;
- trace détaillée du Backtracking ;
- analyse des complexités ;
- implémentation C séquentielle ;
- implémentation C parallèle OpenMP ;
- résultats expérimentaux ;
- diagrammes de Gantt ;
- présentation PowerPoint.
