// Nodo.h (updated)
#ifndef NODO_H
#define NODO_H

template<class T>
class Nodo {
private:
    T dato;
    Nodo<T> *siguiente;

public:
    // Default constructor
    Nodo() : siguiente(nullptr) {}

    // Constructor with data and next pointer
    Nodo(const T& d, Nodo<T>* sig = nullptr) : dato(d), siguiente(sig) {}

    T getDato() {
        return dato;
    }

    void setDato(T d) {
        dato = d;
    }

    Nodo<T> *getSiguiente() {
        return siguiente;
    }

    void setSiguiente(Nodo<T> *siguiente) {
        this->siguiente = siguiente;
    }
};

#endif // NODO_H