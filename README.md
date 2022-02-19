# IPK - Server

Leighweight server, který slouží pro získání základních informací systémových informací ze zadání.

Server je určený pro Linux Ubuntu 20.04 LTS  (https://ubuntu.com/).

## Spouštění

Projekt přeložíme pomocí Makefilu.

```
make
```

Po úspěšném přeložení projektu se vytvoří spustitelný soubor hinfosvc.

Program spustíme:

```
./hinfosvs [port]
```

nebo pro spuštění na pozadí:

```
./hinfosvs [port] &
```

Například tedy:

```
make
./hinfosvs 12345
```


## Využití

Server umí zpracovat tři typy dotazů. Pokud obdrží jiný, vyhodí error 400 Bad request.

Komunikace se serverem je možná pomocí webového prohlížeče, nebo pomocí nastrůjů typu wget a curl.

* 1. Získání doménového jména

Vrací síťové jméno počítače včetně domény, například:

```
GET http://servername:12345/hostname
```
Odpověď:
```
merlin.fit.vutbr.cz
```

* 2. Získání informací o CPU 

Vrací síťové jméno počítače včetně domény, například:

```
GET http://servername:12345/cpu-name
```
Odpověď:
```
Intel(R) Xeon(R) CPU E5-2640 0 @ 2.50GHz
```