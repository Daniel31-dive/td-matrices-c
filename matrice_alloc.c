/*
 * matrice_alloc.c -- Création et destruction des matrices.
 *
 * Exercice 1 : complété (étapes 1.c à 1.f).
 *
 * Représentation : coef est un tableau de nb_lignes pointeurs ; chaque
 * coef[i] pointe vers un tableau de nb_colonnes int alloué séparément.
 */
#include <stdlib.h>
#include "matrice_alloc.h"

/* ------------------------------------------------------------------
 * FOURNI -- Ne pas modifier.
 * mat_calloc s'utilise exactement comme calloc. Compilé avec l'option
 * -DMAT_TEST_ALLOC, il permet aux programmes de test de simuler un
 * manque de mémoire.
 * ------------------------------------------------------------------ */
#ifdef MAT_TEST_ALLOC
int mat_test_allocs_autorisees = -1;

static void *mat_calloc(size_t nb, size_t taille)
{
    if (mat_test_allocs_autorisees == 0)
        return NULL;                       /* échec simulé */
    if (mat_test_allocs_autorisees > 0)
        mat_test_allocs_autorisees--;
    return calloc(nb, taille);
}
#else
#define mat_calloc calloc
#endif

MatErreur mat_allouer(Matrice *res, int nb_lignes, int nb_colonnes)
{
    /* FOURNI : on vérifie res, puis on le met à l'état vide (règle R3). */
    if (res == NULL)
        return MAT_ERR_ARGUMENT;
    *res = MATRICE_VIDE;

    if (nb_lignes < 1 || nb_lignes > MAT_DIM_MAX
        || nb_colonnes < 1 || nb_colonnes > MAT_DIM_MAX)
        return MAT_ERR_ARGUMENT;


    /* FOURNI : tableau des nb_lignes pointeurs de lignes (tous à NULL). */
    int **coef = mat_calloc((size_t)nb_lignes, sizeof *coef);
    if (coef == NULL)
        return MAT_ERR_ALLOCATION;

    /* (1.d) Allocation de chaque ligne ; en cas d'échec on rend tout. */
    for (int i = 0; i < nb_lignes; i++) {
        coef[i] = mat_calloc((size_t)nb_colonnes, sizeof *coef[i]);
        if (coef[i] == NULL) {
            for (int k = i - 1; k >= 0; k--)   /* lignes déjà obtenues */
                free(coef[k]);
            free(coef);                        /* puis le tableau      */
            return MAT_ERR_ALLOCATION;
        }
    }

    /* (1.e) Tout a réussi : on transfère la propriété à l'appelant. */
    res->coef = coef;
    res->nb_lignes = nb_lignes;
    res->nb_colonnes = nb_colonnes;
    return MAT_OK;
}

void mat_detruire(Matrice *m)
{
    if (m == NULL)                 /* FOURNI */
        return;

    /* (1.f) D'abord chaque ligne, PUIS le tableau de pointeurs. */
    if (m->coef != NULL) {
        for (int i = 0; i < m->nb_lignes; i++)
            free(m->coef[i]);
        free(m->coef);
    }


    *m = MATRICE_VIDE;   /* FOURNI : état sûr après destruction (R6) */
}
