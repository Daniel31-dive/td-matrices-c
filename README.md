# TD — Stockage de matrices en C

**Module** : Langage C — 3ᵉ année STI, INSA Centre Val de Loire — année 2026-2027
**Auteurs** : *[Prénom NOM]*, *[Prénom NOM]* (, *[Prénom NOM]*)
**Enseignant** : A. Adell

Petite bibliothèque C de gestion de matrices d'entiers (création, lecture/écriture dans un fichier texte, affichage, addition, produit), en programmation modulaire, avec gestion d'erreurs uniforme et sans fuite mémoire.

## Sommaire

1. [Organisation du dépôt](#1-organisation-du-dépôt)
2. [Compilation et exécution](#2-compilation-et-exécution)
3. [Exercice 1 — Allocation et destruction](#3-exercice-1--allocation-et-destruction)
4. [Exercice 2 — Entrées-sorties](#4-exercice-2--entrées-sorties)
5. [Exercice 3 — Calcul matriciel](#5-exercice-3--calcul-matriciel)
6. [Exercice 4 — Démonstration complète](#6-exercice-4--démonstration-complète)
7. [Validation finale](#7-validation-finale)
8. [Répartition du travail](#8-répartition-du-travail)

---

## 1. Organisation du dépôt

```
.
├── matrice.h / matrice.c              type Matrice, codes d'erreur, outils communs
├── matrice_alloc.h / matrice_alloc.c  mat_allouer, mat_detruire
├── matrice_io.h / matrice_io.c        mat_lire, mat_ecrire, mat_afficher
├── matrice_calcul.h / matrice_calcul.c  mat_additionner, mat_multiplier
├── main.c                             programme de démonstration
├── test_ex1.c, test_ex2.c, test_ex3.c tests fournis
├── donnees/                           A.txt, B.txt, C.txt, invalide.txt, tronque.txt
├── Makefile
└── README.md                          ce compte rendu
```

**Représentation.** Une `Matrice` contient `int **coef` (tableau de `nb_lignes` pointeurs, chacun vers un tableau de `nb_colonnes` entiers alloué séparément), `nb_lignes` et `nb_colonnes`. Une matrice sans mémoire vaut `MATRICE_VIDE`.

**Gestion d'erreurs et propriété mémoire.** Toute fonction pouvant échouer renvoie un `MatErreur` ; le résultat passe par un paramètre de sortie `Matrice *res` ; les entrées sont `const Matrice *` ; la bibliothèque n'affiche rien, c'est le programme appelant qui choisit. Règles R1 à R7 de l'énoncé : initialisation à `MATRICE_VIDE`, transfert de propriété en cas de succès, `*res` vide et rien à libérer en cas d'échec, entrées jamais modifiées, `mat_detruire` idempotent, fichiers fermés par celui qui les ouvre.

## 2. Compilation et exécution

```bash
# Avec le Makefile
make            # compile demo_matrices et les 3 programmes de test
make tests      # lance test_ex1, test_ex2 et test_ex3

# Ou à la main (compilation séparée puis édition de liens)
gcc -std=c11 -Wall -Wextra -Wpedantic -g -c matrice.c matrice_alloc.c matrice_io.c matrice_calcul.c main.c
gcc main.o matrice.o matrice_alloc.o matrice_io.o matrice_calcul.o -o demo_matrices

# Démonstration
./demo_matrices add donnees/A.txt donnees/B.txt R.txt
```

Vérification des fuites : `valgrind --leak-check=full ./test_ex1` (idem `test_ex2`, `test_ex3`, `demo_matrices`).

---

## 3. Exercice 1 — Allocation et destruction

*Fichiers : `matrice.c`, `matrice_alloc.c`.*

### Ce qui a été écrit

| Étape | Travail |
|---|---|
| 1.a | `mat_message_erreur` : ajout des messages pour `MAT_ERR_DIMENSIONS` et `MAT_ERR_DEPASSEMENT` |
| 1.b | `mat_est_valide` : une seule expression avec `&&` (non `NULL`, `coef` non `NULL`, dimensions dans `[1, MAT_DIM_MAX]`) |
| 1.c | `mat_allouer` : dimensions hors `[1, MAT_DIM_MAX]` → `MAT_ERR_ARGUMENT` |
| 1.d | allocation de chaque ligne avec `calloc` ; en cas d'échec, libération des lignes déjà obtenues (de `i-1` à `0`) puis du tableau `coef` |
| 1.e | remplissage de `*res` (3 affectations) seulement une fois tout réussi |
| 1.f | `mat_detruire` : libère chaque ligne **puis** le tableau `coef` ; la matrice est remise à vide |

Extrait (1.d) :

```c
for (int i = 0; i < nb_lignes; i++) {
    coef[i] = mat_calloc((size_t)nb_colonnes, sizeof *coef[i]);
    if (coef[i] == NULL) {
        for (int k = i - 1; k >= 0; k--)
            free(coef[k]);
        free(coef);
        return MAT_ERR_ALLOCATION;
    }
}
```

### Résultats

`./test_ex1` → `Bilan : 16 / 16 verifications reussies.` Aucune fuite détectée (AddressSanitizer).

### Réponses aux questions

**Q3.1 — Pourquoi `Matrice *` et non `Matrice` ?**
Avec un passage par valeur, la fonction recevrait une *copie* de la structure. Elle libérerait bien la mémoire, mais remettrait à `MATRICE_VIDE` seulement sa copie : la variable de l'appelant garderait un pointeur `coef` devenu invalide (pointeur pendant). Toute utilisation ultérieure, ou un second `mat_detruire`, serait un accès ou une libération invalide. Avec un pointeur, la fonction modifie la matrice de l'appelant elle-même.

**Q3.2 — Combien d'appels à `calloc` et `free` ? Et si la k-ième allocation échoue ?**
Pour une matrice n × p : 1 + n appels à `calloc` (le tableau de pointeurs, puis une ligne par `i`) et 1 + n appels à `free`. Si la k-ième allocation échoue, `mat_allouer` fait k − 1 libérations : pour k = 1 (le tableau `coef` lui-même) il n'y a rien à libérer ; pour k ≥ 2 elle libère les k − 2 lignes déjà obtenues plus le tableau `coef`.

**Q3.3 — Pourquoi deux appels à `mat_detruire` sont sans danger ? Et avec une copie `Matrice b = a;` ?**
Après le premier appel, la matrice vaut `MATRICE_VIDE` : `coef == NULL`, donc le second appel ne fait rien (règle R6). Ce n'est **plus vrai** avec une copie : `b = a` est une copie superficielle, `b.coef` contient la même adresse. `mat_detruire(&a)` libère la mémoire et remet seulement `a` à vide ; `b.coef` pointe toujours vers la zone libérée, donc `mat_detruire(&b)` effectue une double libération (comportement indéfini).

**Q3.4 — Pourquoi `*res = MATRICE_VIDE` dès le début ?**
Pour que *toutes* les sorties anticipées (argument invalide, échec d'allocation) laissent `*res` dans un état défini et sûr (règle R3). Sans cela, `*res` pourrait contenir une valeur indéterminée (variable non initialisée) ou l'ancienne matrice de l'appelant, que celui-ci risquerait d'utiliser ou de détruire à tort.

### Validation

- [x] `test_ex1` compile sans avertissement
- [x] `Bilan : 16 / 16`
- [x] aucune fuite (AddressSanitizer ; à reconfirmer avec valgrind sur votre machine)
- [x] plus aucune ligne provisoire dans `matrice.c` et `matrice_alloc.c`

---

## 4. Exercice 2 — Entrées-sorties

*Fichier : `matrice_io.c`.*

### Ce qui a été écrit

| Étape | Travail |
|---|---|
| 2.a | `mat_afficher` : double boucle, `printf(" %8d", …)` puis `putchar('\n')` à chaque ligne |
| 2.b | `mat_ecrire` : format canonique (`"%d"` pour le premier coefficient, `" %d"` pour les suivants), `fputc('\n')` par ligne, chaque écriture testée → `MAT_ERR_ECRITURE` |
| 2.c | `mat_lire` : dimensions hors bornes → `MAT_ERR_FORMAT` (le contenu du fichier est faux, pas les paramètres) |
| 2.d | allocation d'une matrice **locale** `tmp` |
| 2.e | lecture des n × p coefficients avec `lire_entier` ; en cas d'échec : `mat_detruire(&tmp)` puis renvoi de l'erreur |
| 2.f | `*res = tmp; return MAT_OK;` |

### Résultats

`./test_ex2` → `Bilan : 14 / 14 verifications reussies.` y compris les six fichiers incorrects et l'échec d'allocation simulé. Aucune fuite.

### Réponses aux questions

**Q4.1 — Pourquoi une matrice locale avant d'affecter `*res` ?**
Pour que `*res` ne soit modifié qu'en cas de succès complet. Si une erreur survient au milieu du fichier (donnée non numérique, fichier tronqué), on détruit `tmp` et `*res` reste `MATRICE_VIDE` : l'appelant ne reçoit jamais une matrice à moitié remplie (règle R3). Cela garantit aussi qu'il n'a rien à libérer en cas d'échec.

**Q4.2 — La lecture vérifie-t-elle qu'une ligne de matrice est sur sa propre ligne de texte ? Est-ce gênant ? Pourquoi « tolérant en lecture, strict en écriture » ?**
Non : `lire_entier` lit des « mots » séparés par n'importe quels blancs (espaces, tabulations, sauts de ligne) ; seul compte le nombre total d'entiers, n × p. Ce n'est pas gênant : la structure est entièrement déterminée par les deux dimensions lues au début, la mise en page est sans importance. Le principe est avantageux : en lecture, tolérer les blancs rend le programme robuste face à des fichiers édités à la main ou produits par d'autres outils ; en écriture, un format canonique unique rend les fichiers produits prévisibles, comparables octet par octet (ce que vérifie le test) et relisibles par n'importe quel programme, même rigide. La tolérance ne porte que sur les blancs : le contenu reste vérifié strictement (dimensions, nombre de coefficients, entiers valides).

**Q4.3 — Stocker deux matrices dans un même fichier ?**
Comme `mat_lire` laisse le flux juste après le dernier coefficient, on peut l'appeler deux fois de suite sur le même `FILE *` : `mat_lire(f, &a)` lit la première, `mat_lire(f, &b)` lit la suivante. Symétriquement, deux appels successifs à `mat_ecrire(f, …)` écrivent les deux matrices l'une après l'autre. C'est possible parce que ces fonctions n'ouvrent ni ne ferment le fichier (règle R7).

**Q4.4 — Pourquoi `fflush` avant de renvoyer `MAT_OK` ? Dispense-t-il de tester `fclose` ?**
Les `fprintf` écrivent dans un tampon ; une erreur (disque plein, périphérique déconnecté) peut n'apparaître qu'au moment où le tampon est réellement vidé. `fflush` force ce vidage *dans* `mat_ecrire`, qui peut donc signaler `MAT_ERR_ECRITURE` au lieu de renvoyer un faux succès. Cela ne dispense pas de tester `fclose` : `fclose` peut lui aussi échouer (dernières écritures, libération de la ressource), et `fflush` ne garantit pas que les données soient arrivées sur le disque. C'est pourquoi `sauvegarder` teste aussi `fclose`.

### Validation

- [x] `test_ex2` compile sans avertissement
- [x] `Bilan : 14 / 14`
- [x] aucune fuite (AddressSanitizer ; à reconfirmer avec valgrind sur votre machine)
- [x] aucun `fopen` / `fclose` dans `matrice_io.c`, plus aucune ligne provisoire

---

## 5. Exercice 3 — Calcul matriciel

*Fichier : `matrice_calcul.c`.*

### Ce qui a été écrit

| Étape | Travail |
|---|---|
| 3.a | addition : mêmes nombres de lignes **et** de colonnes, sinon `MAT_ERR_DIMENSIONS` |
| 3.b | allocation du résultat avec les dimensions de `a` |
| 3.c | somme en `long long` : `(long long)a->coef[i][j] + b->coef[i][j]` |
| 3.d | `*res = r; return MAT_OK;` |
| 3.e | produit : colonnes de `a` = lignes de `b`, sinon `MAT_ERR_DIMENSIONS` |
| 3.f | allocation du résultat `(lignes de a) × (colonnes de b)` |
| 3.g | boucle sur `k` : `s += a(i,k) × b(k,j)` avec contrôle de dépassement de `long long` avant chaque addition |
| 3.h | transfert du résultat |

Extrait (3.g) :

```c
for (int k = 0; k < a->nb_colonnes; k++) {
    long long p = (long long)a->coef[i][k] * b->coef[k][j];
    if ((p > 0 && s > LLONG_MAX - p) || (p < 0 && s < LLONG_MIN - p)) {
        mat_detruire(&r);
        return MAT_ERR_DEPASSEMENT;
    }
    s += p;
}
```

### Résultats

`./test_ex3` → `Bilan : 14 / 14 verifications reussies.` Aucune fuite.

### Réponses aux questions

**Q5.1 — `const Matrice *a` empêche-t-il `a->coef[0][0] = 7;` ?**
Non, cette affectation est **acceptée** par le compilateur (nous l'avons vérifié). Le `const` rend en lecture seule la *structure* pointée : `a->nb_lignes = 7;` ou `a->coef = NULL;` sont refusés. Mais le champ `coef` est de type `int ** const` : le pointeur lui-même est constant, pas les `int` vers lesquels il mène, qui sont dans des blocs distincts alloués sur le tas. La protection est donc « superficielle ». Pour que le compilateur refuse aussi l'écriture des coefficients, il faudrait déclarer le champ `const int *const *coef` (et adapter l'allocation, qui devrait alors écrire via un pointeur non constant), ou ne pas exposer `coef` directement et passer par des accesseurs.

**Q5.2 — Pourquoi refuser `mat_additionner(&a, &b, &a)` ?**
Notre implémentation écrit le résultat dans `*res` après avoir calculé dans une matrice `r` distincte. Si `res == a` : `verifier_parametres` met d'abord `*res` à `MATRICE_VIDE`, donc **`a` est détruite avant d'être lue** (les `mat_est_valide` suivants échouent, ou on lirait une matrice vide) ; et même sans cela, l'affectation finale `*res = r` écraserait le pointeur `coef` de `a` sans libérer l'ancienne mémoire : fuite. Pour le produit, le calcul en place est de toute façon impossible, car chaque ligne de `a` est relue pour calculer toute une ligne de résultat. Refuser l'aliasing (`MAT_ERR_ARGUMENT`, rien n'est modifié) est la solution simple et sûre ; l'appelant peut faire `r = a + b` puis détruire `a` et réaffecter.

**Q5.3 — Nombre de multiplications ?**
Pour A (n × p) et B (p × q) : n × q coefficients, chacun nécessitant p multiplications, donc **n·p·q** multiplications. Pour deux matrices 1000 × 1000 : 1000³ = **10⁹** multiplications (un milliard), autant d'additions ; la complexité est cubique pour des matrices carrées.

**Q5.4 — Pourquoi `int s = a + b;` puis `s < a` ne suffit pas ?**
Parce que le dépassement d'un `int` signé est un **comportement indéfini** : une fois que `a + b` a débordé, on ne peut plus raisonner sur la valeur de `s`. Le compilateur est même autorisé à supposer que le dépassement n'arrive jamais, et peut donc supprimer le test lors de l'optimisation. De plus, le test `s < a` n'est valable que si `b > 0` ; pour `b < 0` le dépassement se fait par le bas et il faut un autre test. La bonne méthode est de calculer dans un type plus large (`long long`) puis de vérifier que le résultat tient dans un `int`.

### Validation

- [x] `test_ex3` compile sans avertissement
- [x] `Bilan : 14 / 14`
- [x] aucune fuite (AddressSanitizer ; à reconfirmer avec valgrind sur votre machine)
- [x] plus aucune ligne provisoire dans `matrice_calcul.c`

---

## 6. Exercice 4 — Démonstration complète

*Fichier : `main.c`.*

### Ce qui a été écrit

| Étape | Travail |
|---|---|
| 4.a | `mat_afficher("A", &a); mat_afficher("B", &b);` |
| 4.b | calcul avec l'opérateur `? :` ; en cas d'erreur, message sur `stderr` avec la cause et les dimensions de A et B, puis `goto nettoyage` |
| 4.c | `mat_afficher(addition ? "R = A + B" : "R = A x B", &r);` |
| 4.d | message de succès et `statut = EXIT_SUCCESS;` |

Le programme a un **point de sortie unique** (`nettoyage:`) qui détruit `r`, `b`, `a`. Comme `mat_detruire` est sans effet sur une matrice vide, il fonctionne quel que soit le chemin d'erreur.

### Les dix scénarios

Tous ont été exécutés (avec AddressSanitizer/LeakSanitizer, qui ne signale ni fuite ni accès invalide).

| # | Commande | Résultat observé | Code |
|---|---|---|---|
| 1 | `add A B R` | A, B, R affichés ; `R.txt` créé au format canonique (R = 26 50 87 / 19 38 17 / 52 96 19 / 10 23 7) | 0 |
| 2 | `mul A C R` | R de dimensions 4 × 2 (193 −36 / 37 22 / 65 78 / −10 17) | 0 |
| 3 | `add A C R` | `addition impossible : dimensions incompatibles (A : 4 x 3, B : 3 x 2)` ; aucun fichier créé | 1 |
| 4 | `mul A B R` | `multiplication impossible : dimensions incompatibles (A : 4 x 3, B : 4 x 3)` | 1 |
| 5 | fichier `absent.txt` | `impossible d'ouvrir "absent.txt" : No such file or directory` | 1 |
| 6 | `invalide.txt` comme A | `lecture de "donnees/invalide.txt" : fichier mal formé` | 1 |
| 7 | `tronque.txt` comme A | `lecture de "donnees/tronque.txt" : fichier mal formé` | 1 |
| 8 | résultat dans `dossier_inexistant/R.txt` | `impossible de créer "dossier_inexistant/R.txt" : No such file or directory` ; matrices libérées | 1 |
| 9 | échec d'allocation | simulé dans `test_ex2` et `test_ex3` : `MAT_ERR_ALLOCATION`, aucune fuite | — |
| 10 | arguments manquants / opération inconnue | message `Usage : …` | 1 |

Relecture de son propre fichier : `./demo_matrices add R.txt R.txt R2.txt` fonctionne (R2 = 52 100 174 / 38 76 34 / 104 192 38 / 20 46 14).

### Réponses aux questions

**Q6.1 — Pourquoi le message d'erreur est-il affiché par `main.c` et non par `mat_lire` ?**
Une bibliothèque ne sait pas dans quel contexte elle est utilisée. Un programme graphique afficherait une boîte de dialogue, un serveur écrirait dans un journal, un programme en ligne de commande écrit sur `stderr`, un autre voudra peut-être réessayer en silence. Si `mat_lire` écrivait elle-même, elle imposerait un comportement (et un canal de sortie) à tous ses utilisateurs et serait difficile à tester. En renvoyant un code (`MatErreur`) traduit par `mat_message_erreur`, la bibliothèque laisse l'appelant décider. De plus, seul l'appelant connaît le contexte utile (nom du fichier, opération en cours).

**Q6.2 — Pourquoi tester `fclose` dans `sauvegarder` mais pas dans `charger` ?**
En lecture, `fclose` ne peut pas faire perdre de données : toutes les données utiles ont déjà été lues et validées, et un échec éventuel de fermeture n'a aucune conséquence sur le résultat. En écriture, `fclose` vide les derniers tampons : c'est souvent là qu'apparaît un disque plein ou une erreur d'écriture différée. Ignorer son résultat ferait croire à un enregistrement réussi alors que le fichier est incomplet.

**Q6.3 — Scénario 8 : combien de matrices à libérer ?**
**Trois** : `a` et `b` (lues avec succès) et `r` (calculée avec succès) possèdent toutes de la mémoire quand l'enregistrement échoue. Le `goto nettoyage` mène au point de sortie unique qui appelle `mat_detruire(&r)`, `mat_detruire(&b)`, `mat_detruire(&a)` : chacune est détruite exactement une fois. Vérifié par AddressSanitizer (aucune fuite).

### Validation

- [x] compilation sans avertissement avec `-std=c11 -Wall -Wextra -Wpedantic`
- [x] `main.c` n'inclut que des en-têtes ; aucun module ne contient de `main`
- [x] les dix scénarios se comportent comme attendu
- [x] le fichier du scénario 1 est relisible par le programme lui-même
- [x] aucune fuite ni accès invalide
- [x] `test_ex1`, `test_ex2`, `test_ex3` affichent toujours un bilan complet

---

## 7. Validation finale

| Programme | Bilan |
|---|---|
| `test_ex1` | 16 / 16 |
| `test_ex2` | 14 / 14 |
| `test_ex3` | 14 / 14 |
| `demo_matrices` | 10 scénarios conformes, 0 avertissement, 0 fuite |

## 8. Répartition du travail

*À compléter par le groupe : qui a fait quoi (exercices, tests, README), et les difficultés rencontrées.*

| Membre | Contribution |
|---|---|
| *[Prénom NOM]* | |
| *[Prénom NOM]* | |
