#ifndef U05_HASH_HASHMAP_HASHMAPLIST_H_
#define U05_HASH_HASHMAP_HASHMAPLIST_H_

#include "HashEntry.h"
#include "Lista.h"
#include <functional>
#include <string>

template <class K, class T>
class HashMapList {
private:
    Lista<HashEntry<K, T>> **tabla;
    unsigned int tamanio;
    static unsigned int hashFunc(K clave);
    unsigned int (*hashFuncP)(K clave);

public:
    explicit HashMapList(unsigned int k);
    HashMapList(unsigned int k, unsigned int (*hashFuncP)(K clave));
    void getList(K clave);
    void put(K clave, T valor);
    void remove(K clave);
    void iterate(std::function<void(K, T&)> func);
    T& get(K clave);
    ~HashMapList();
    bool esVacio();
    void print();
};

template <class K, class T>
HashMapList<K, T>::HashMapList(unsigned int k) {
    tamanio = k;
    tabla = new Lista<HashEntry<K, T>> *[tamanio];
    for (int i = 0; i < tamanio; i++) {
        tabla[i] = nullptr;
    }
    hashFuncP = hashFunc;
}

template <class K, class T>
HashMapList<K, T>::HashMapList(unsigned int k, unsigned int (*fp)(K)) {
    tamanio = k;
    tabla = new Lista<HashEntry<K, T>> *[tamanio];
    for (int i = 0; i < tamanio; i++) {
        tabla[i] = nullptr;
    }
    hashFuncP = fp;
}

template <class K, class T>
HashMapList<K, T>::~HashMapList() {
    for (int i = 0; i < tamanio; i++) {
        if (tabla[i] != nullptr) {
            delete tabla[i];
        }
    }
    delete[] tabla;
}

template <class K, class T>
void HashMapList<K, T>::put(K clave, T valor) {
    unsigned int pos = hashFuncP(clave) % tamanio;
    if (tabla[pos] == nullptr) {
        tabla[pos] = new Lista<HashEntry<K, T>>();
    }

    Nodo<HashEntry<K, T>>* nodo = tabla[pos]->getInicio();
    while (nodo != nullptr) {
        if (nodo->getDato().getClave() == clave) {
            nodo->getDato().setValor(valor);  // ACTUALIZA si ya existe
            return;
        }
        nodo = nodo->getSiguiente();
    }

    tabla[pos]->insertarUltimo(HashEntry<K, T>(clave, valor));  // Si no estaba, lo agrega
}

template <class K, class T>
void HashMapList<K, T>::remove(K clave) {
    unsigned int pos = hashFuncP(clave) % tamanio;
    if (tabla[pos] == nullptr) {
        throw 404;
    }

    Nodo<HashEntry<K, T>>* nodo = tabla[pos]->getInicio();
    int posicion = 0;

    while (nodo != nullptr) {
        if (nodo->getDato().getClave() == clave) {
            tabla[pos]->remover(posicion);
            if (tabla[pos]->esVacia()) {
                delete tabla[pos];
                tabla[pos] = nullptr;
            }
            return;
        }
        nodo = nodo->getSiguiente();
        posicion++;
    }

    throw 404;
}


template <class K, class T>
bool HashMapList<K, T>::esVacio() {
    for (int i = 0; i < tamanio; i++) {
        if (tabla[i] != nullptr) {
            return false;
        }
    }
    return true;
}

template <class K, class T>
unsigned int HashMapList<K, T>::hashFunc(K clave) {
    if constexpr (std::is_same_v<K, std::string>) {
        std::hash<std::string> hash_fn;
        return hash_fn(clave);
    } else {
        return static_cast<unsigned int>(clave);
    }
}

template <class K, class T>
void HashMapList<K, T>::getList(K clave) {
    unsigned int pos = hashFuncP(clave) % tamanio;
    if (tabla[pos] == nullptr) {
        throw 404;
    }
    Nodo<HashEntry<K, T>> *aux = tabla[pos]->getInicio();
    while (aux != nullptr) {
        std::cout << aux->getDato().getValor() << std::endl;
        aux = aux->getSiguiente();
    }
}

template <class K, class T>
void HashMapList<K, T>::iterate(std::function<void(K, T&)> func) {
    for (int i = 0; i < tamanio; i++) {
        if (tabla[i] != nullptr) {
            Nodo<HashEntry<K, T>>* nodo = tabla[i]->getInicio();
            while (nodo != nullptr) {
                func(nodo->getDato().getClave(), nodo->getDato().getValor());
                nodo = nodo->getSiguiente();
            }
        }
    }
}

template <class K, class T>
T& HashMapList<K, T>::get(K clave) {
    unsigned int pos = hashFuncP(clave) % tamanio;

    if (tabla[pos] == nullptr) {
        throw 404;
    }

    Nodo<HashEntry<K, T>>* nodo = tabla[pos]->getInicio();
    while (nodo != nullptr) {
        if (nodo->getDato().getClave() == clave) {
            return nodo->getDato().getValor();
        }
        nodo = nodo->getSiguiente();
    }

    throw 404;
}




#endif // U05_HASH_HASHMAP_HASHMAPLIST_H_