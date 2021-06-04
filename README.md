# Traitement numérique des images - Devoir 4

## Dépendences

Fait sur Linux, Makefile fournit.

- build-essential
- libboost-all-dev

## Construction

```sh
make
```

## Utilisation

Pour compresser :

```sh
./lossless-codec -c -i images/034.ppm -o a.blp -p A
```

Environ 10 min d'exécution sur ma machine.

Pour decompresser :

```sh
./lossless-codec -d -i a.blp -o a.ppm
```

## Compress A

Inspiré de CALIC.

## Compress B

Inspiré d'ADAM7. C'est donc prédicteur progressif.

## Compress C

Parcourt l'image en zigzag de haut en bas. Utilise le dernier pixel visité comme prédiction. Essaye de mitiger le problème de l'algorithme prédictif "de droite". En effet, en parcourant l'image de gauche à droite, cela créer un "edge" artificiel au rebord de l'image. Le zigzag évite ce problème. 