# xsh
A simple shell written in C that supports redirection, pipe, background job, comments.

## Installation
```
make
```

## Usage
```
./xsh
```

## Example
```
./xsh
? cat main.c | wc -w > temp
? cat temp
14
? rm temp
? 
```
