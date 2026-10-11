#ifndef GRAPH_H
#define GRAPH_H

int get_m3u8_url(char *source_url, char *out_url, int out_url_taille); // Récupération du lien du flux vidéo
void calculVoisinage(int width, int height, int chunk, int pixel, int *sortie); // sortie est de taille (2*chunk+1)²
void calculIntensitePonctuel(int width, int height, int pixel, uint8_t *matrice, uint8_t *matrice_ancienne, float *sortie);
void calculFluxPonctuel(int width, int height, int chunk, int pixel, uint8_t *matrice, uint8_t *matrice_ancienne, float *sortie);
void dessiner(int width, int height, float *vecteur, int pixel, uint32_t *matrice_affiche);
int direction_vecteur(float *vecteur);

#endif