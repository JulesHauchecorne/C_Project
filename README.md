# TP1 - pf le cascadeur

L'objectif du TP est de développer un utilitaire `pf` qui mesure les ressources utilisés par une commande.

## Usage

`pf [-u|-c|-a] [-n n] [-s] commande [argument...]`

L’utilitaire exécute une commande, éventuellement plusieurs fois en parallèle et/ou en série, et affiche le temps réel et/ou utilisateur consommé par la commande et/ou le nombre d'instructions machines utilisés par la commande.

C'est une version simplifiée des utilitaires `time` et `perf`.

###

Par défaut, `pf` présente le nombre de secondes écoulées en temps réel par la commande (deux chiffre de précision après le point).
L'affichage se fait sur la sortie standard d'erreur.

```
$ ./pf ./work
1.00
```

Ceci est analogue à ce que retourne la commande `time` avec `%e`.

```
$ command time -f "%e" ./work
1.00
```

Bien sur, la commande peut être n'importe quel programme valide et ses arguments.

```
$ ./pf sleep 1.5
1.50
```

Attention, les options de `pf` apparaissent nécessairement avant la commande. Si une option apparait après la commande elle considérée comme un argument de la commande.

### -u

Avec l'option `-u`, `pf` présente le nombre de secondes écoulées en temps utilisateur par la commande (deux chiffre de précision après le point)

```
$ ./pf -u ./work
0.66
$ ./pf -u sleep 1
0.00
```

Ceci est analogue à ce que retourne la commande `time` avec `%u`.

```
$ command time -f "%U" ./work
0.66
$ command time -f "%U" sleep 1
0.00
```

### -c

Avec l'option `-c`, `pf` présenter le nombre de cycles processeur consommés par la commande en mode utilisateur.

```
$ ./pf -c ./work
140938200
```

Ceci est analogue au compteur `cycles` de `perf stat`. C'est le premier nombre affiché par la commande de l'exemple suivant.

```
$ perf stat -e cycles -x, ./work
142304586,,cycles:u,1000201099,100,00,,
```


Pour les détails, man `perf stat`. `perf` est un outil puissant offert par le noyau Linux.
Je vous invite à regarder ce qu'il est possible de faire avec.

### -a

Avec l'option `-a`, l'utilitaire `pf` affiche les trois valeurs : le temps réel, un espace, le temps utilisateur, un espace, et le nombre de cycles processeurs.

```
$ ./pf -a ./work
1.00 0.61 143603651
$ ./pf -a sleep 1.1
1.10 0.00 1211159
```


### -n

L'option `-n` indique un nombre de répétition.
La commande est ainsi exécutée autant de fois qu'indiqué avec un affichage des valeurs de chaque exécution.

Si n est supérieur à 1, une ligne supplémentaire est affichée en dernier qui contient les valeurs moyennes de l'ensemble des exécutions.

```
./pf -n 5 ./work
1.00
1.00
0.99
1.00
1.00
1.00
```

Les commandes sont exécutées l'une à la suite de l'autre.
Ainsi l'exemple précédent a pris 5 secondes de temps réel pour s'exécuter.

On pourrait utiliser `time` pour mesurer le temps réel disponible mais `./pf` fait aussi l'affaire.
Par exemple, un premier `pf` exécute et mesure un second `pf` qui exécute deux `sleep 1` :

```
./pf ./pf -n 2 sleep 1
1.00
1.00
1.00
2.00
```



Bien évidemment, `-n` est compatible avec les autres options de l'outil.

```
./pf -a -n 3 sleep 1.1
1.10 0.00 454595
1.10 0.00 441480
1.10 0.00 430592
1.10 0.00 442222
```

### -s

L'option `-s` exécute la commande dans un sous shell `/bin/sh` via l'option `-c`.
Cela permet de mesurer des commandes complexes.
Bien sur, le cout du shell est mesuré aussi.

```
./pf -s './work; sleep .5'
1.50
```

Bien évidemment, `-s` est compatible avec les autres options de l'outil.

```
./pf -a -n 3 -s './work& ./work; wait'
1.00 1.32 278797959
1.00 1.34 278431654
1.00 1.33 275326470
1.00 1.33 277518694
```



### Valeur de retour

La valeur de retour est celle de la dernière commande qui a fini de s'exécuter.

S'il y a un problème avec l'exécution de la commande, `pf` affiche un message d'erreur et termine avec une valeur de retour de 127.


## Contraintes d'implémentation

Le parallélisme se fera avec des fork et non avec des threads.

Pour mesurer le temps réel, vous utiliserez l'appel système POSIX `clock_gettime` avec l'horloge `CLOCK_MONOTONIC`.

Pour déterminer le temps utilisateur consommé, vous utiliserez l'appel système Linux `wait4`.

Pour déterminer le nombre de cycles CPU utilises, vous utiliserez l'appel système Linux `perf_event_open` et le compteur `PERF_COUNT_HW_CPU_CYCLES`.

Attention, pour fonctionner, il est important d'avoir un Linux pas trop ancien (à partir de 2.6.32, décembre 2009) et que le noyau soit compilé et configuré pour permettre l'utilisation de `perf_event_open` ce qui est le cas de la plupart des distributions Linux actuelles.
Vérifiez que `/proc/sys/kernel/perf_event_paranoid` existe et a une valeur inférieure ou égale à 2. Dans le cadre du TP, une valeur 2 (qui est celle par défaut) est suffisante, une valeur plus faible permet d’accéder à des informations plus bas niveau non nécessaire pour le TP mais toujours intéressante à explorer.


## Réalisation

Vous devez développer le programme `pf` en C.
Le fichier source doit s'appeler `pf.c` et être à la racine du dépôt.
Vu la taille du projet, tout doit rentrer dans ce seul fichier source.

Pour pouvez compiler avec `make` (le `Makefile` est fourni).

Vous pouvez vous familiariser avec le contenu du dépôt, en étudiant chacun des fichiers (`README.md`, `Makefile`, `check.bats`, `.gitlab-ci.yml`, etc.).

⚠️ À priori, il n'y a pas de raison de modifier un autre fichier du dépôt.
Si vous en avez besoin, ou si vous trouvez des bogues ou problèmes dans les autres fichiers, merci de me contacter.
Un bonus sera attribué si vous implémentez une option `-p` pertinente.


## Acceptation et remise du TP

### Acceptation

Une interface web vous permet d'accepter le TP:

* [Interface web *travo*](https://info.pages.info.uqam.ca/travo-web/?project=1961)

Autrement, vous pouvez accepter le TP manuellement en faisant les trois actions directement:

* Cloner (fork) ce dépôt sur le gitlab départemental.
* Le rendre privé : dans `Settings` → `General` → `Visibility` → `Project visibility` → `Private`.
* Ajouter l'utilisateur `@privat` comme mainteneur (oui, j'ai besoin de ce niveau de droits) : dans `Settings` → `Members` → `Invite member` → `@privat` (n'invitez pas @privat2, 3 ou 4 : ce sont mes comptes de tests).
* ⚠️ Mal effectuer ces étapes vous expose à des pénalités importantes.


### Remise

La remise s'effectue simplement en poussant votre code sur la branche `master` de votre dépôt gitlab privé.
Seule la dernière version disponible avant le **dimanche 7 mars 23 h 55** sera considérée pour la correction.


### Intégration continue et mises à jour

⚠️ **Important** L'infrastructure de test n'est pas encore finalisée :

* L'infrastructure de test est paranoïaque par défaut et rend difficile l'exécution d'un programme qui utilise `perf_event_open` : donc les tests échouent passent chez vous mais échouent sur gitlab.
* Les tests réalisés actuellement sont très superficiels mais on va essayer de les améliorer : vérifier du comportement temporel est complexe à automatiser car les résultats varient d'une machine à l'autre et d'une exécution à l'autre.
* Il est ultimement de votre responsabilité de tester et valider votre programme.

Le système d'intégration continue vérifie votre TP à chaque `push`.
Vous pouvez vérifier localement avec `make check` (l'utilitaire `bats` entre autres est nécessaire).

Les tests fournis ne couvrent que les cas d'utilisation de base, en particulier ceux présentés ici.
Il est **fortement suggéré** d'ajouter vos propres tests dans [local.bats](local.bats) et de les pousser pour que l’intégration continue les prennent en compte.
Ils sont dans un job distincts pour avoir une meilleure vue de l'état du projet.

❤ Des points bonus pourront être attribués si `local.bats` contient des tests pertinents et généralisables.

❤ En cas de problème pour exécuter les tests sur votre machine, merci de 1. lire la documentation présente ici et 2. poser vos questions sur [/opt/tp1](https://mattermost.info.uqam.ca/inf3173-h21/channels/tp1).
Attention toutefois à ne pas fuiter de l’information relative à votre solution (conception, morceaux de code, etc.)

### Critères de correction

Seront considéré pour la correction du code: l'exactitude, la robustesse, la lisibilité, la simplicité, la conception, les commentaires, etc.

Une pénalité sera appliquée pour chacun des tests publics (`make check`) qui ne passent pas l'intégration continue du gitlab.

Comme le TP n'est pas si gros (de l'ordre de grandeur d'une ou deux centaines de lignes), il est attendu un effort important sur le soin du code et la gestion des cas d'erreurs.

## Mentions supplémentaires importantes

⚠️ **Intégrité académique**
Rendre public votre dépôt personnel ou votre code ici ou ailleurs ; ou faire des MR contenant votre code vers ce dépôt principal (ou vers tout autre dépôt accessible) sera considéré comme du **plagiat**.

⚠️ Attention, vérifier **≠** valider.
Ce n'est pas parce que les tests passent chez vous ou ailleurs ou que vous avez une pastille verte sur gitlab que votre TP est valide et vaut 100%.
Par contre, si des tests échouent quelque part, c'est généralement un bon indicateur de problèmes dans votre code.

⚠️ Si votre programme **ne compile pas** ou **ne passe aucun test**, une note de **0 sera automatiquement attribuée**, et cela indépendamment de la qualité de code source ou de la quantité de travail mise estimée.


Quelques exemples de pénalités additionnelles:

* Vous faites une MR sur le dépôt public avec votre code privé : à partir de -5%
* Vous demandez à être membre du dépôt public : -5%
* Si vous critiquez à tord l'infrastructure de test : -5%
* Vous modifiez un fichier autre que le fichier source ou `local.bats` (ou en créez un) sans avoir l’autorisation : à partir de -10%
* Votre dépôt n'est pas un fork de celui-ci : -100%
* Votre dépôt n'est pas privé : -100%
* L'utilisateur @privat n'est pas mainteneur : -100%
* Votre dépôt n'est pas hébergé sur le gitlab départemental : -100%
* Vous faites une remise par courriel : -100%
* Vous utilisez « mais chez-moi ça marche » (ou une variante) comme argument : -100%
* Si je trouve des morceaux de votre code sur le net (même si vous en êtes l'auteur) : -100%
