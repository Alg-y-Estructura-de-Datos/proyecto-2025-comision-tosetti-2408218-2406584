#ifndef U04_COLAS_COLA_COLA_PRIORIDAD_H_
#define U04_COLAS_COLA_COLA_PRIORIDAD_H_

#include "nodoPrioridad.h"

#define SIN_PRIORIDAD 1000000

/**
 * Clase que implementa una Cola generica, ya que puede
 * almacenar cualquier tipo de dato T
 * @tparam T cualquier tipo de dato
 */
template <class T>
class ColaPrioridad
{
private:
  NodoPrioridad<T> *tope, *fondo;

public:
  ColaPrioridad();

  ~ColaPrioridad();

  void encolar(T dato);

  void encolarPrioridad(T dato, int prioridad);

  T desencolar();

  bool esVacia();

  T peek();

  int size(); // Added method to return the number of elements in the queue
};

/**
 * Constructor de la clase Cola
 * @tparam T
 */
template <class T>
ColaPrioridad<T>::ColaPrioridad()
{
  tope = nullptr;
  fondo = nullptr;
}

/**
 * Destructor de la clase Cola, se encarga de liberar la memoria de todos los
 * nodos utilizados en la Cola
 * @tparam T
 */
template <class T>
ColaPrioridad<T>::~ColaPrioridad()
{
  while (!esVacia())
  {
    desencolar();
  }

  delete tope;
  delete fondo;
}

/**
 * Inserta un dato en la Cola
 * @tparam T
 * @param dato  dato a insertar
 */
template <class T>
void ColaPrioridad<T>::encolar(T dato)
{
  NodoPrioridad<T> *nuevo = new NodoPrioridad<T>();
  nuevo->setDato(dato);
  nuevo->setSiguiente(nullptr);
  nuevo->setPrioridad(SIN_PRIORIDAD);

  if (esVacia())
  {
    tope = nuevo;
  }
  else
  {
    fondo->setSiguiente(nuevo);
  }

  fondo = nuevo;
}

/**
 * Obtener el dato de la Cola
 * @tparam T
 * @return dato almacenado en el nodo
 */
template <class T>
T ColaPrioridad<T>::desencolar()
{
  if (esVacia())
  {
    throw 400;
  }

  T dato = tope->getDato();
  NodoPrioridad<T> *aBorrar = tope;
  tope = tope->getSiguiente();

  if (tope == nullptr)
  {
    fondo = nullptr;
  }

  delete aBorrar;
  return dato;
}

/**
 * Responde si la Cola se encuentra Vacía
 * @tparam T
 * @return
 */
template <class T>
bool ColaPrioridad<T>::esVacia()
{
  return fondo == nullptr;
}

/**
 * Obtener el dato de la Cola sin eliminar el nodo
 * @tparam T
 * @return dato almacenado en el nodo
 */
template <class T>
T ColaPrioridad<T>::peek()
{
  if (esVacia())
  {
    throw 400;
  }

  return tope->getDato();
}

/**
 * Inserta un dato en la Cola con una prioridad específica
 * @tparam T
 * @param dato  dato a insertar
 * @param prioridad  prioridad del dato
 */
template <class T>
void ColaPrioridad<T>::encolarPrioridad(T dato, int prioridad)
{
  if (prioridad >= SIN_PRIORIDAD)
  {
    encolar(dato);
    return;
  }

  NodoPrioridad<T> *nuevo = new NodoPrioridad<T>();
  nuevo->setDato(dato);
  nuevo->setPrioridad(prioridad);

  NodoPrioridad<T> *aux = tope;
  
  if (esVacia())
  {
    tope = nuevo;
    fondo = tope;
    return;
  }
  
  if (tope->getPrioridad() > prioridad)
  {
    nuevo->setSiguiente(tope);
    tope = nuevo;
    return;
  }

  while (aux->getSiguiente() != nullptr && aux->getSiguiente()->getPrioridad() <= prioridad)
  {
    aux = aux->getSiguiente();
  }

  nuevo->setSiguiente(aux->getSiguiente());
  aux->setSiguiente(nuevo);
  if (nuevo->getSiguiente() == nullptr)
  {
    fondo = nuevo;
  }
}

/**
 * Devuelve el número de elementos en la Cola
 * @tparam T
 * @return número de elementos
 */
template <class T>
int ColaPrioridad<T>::size()
{
  int count = 0;
  NodoPrioridad<T> *current = tope;
  while (current != nullptr)
  {
    count++;
    current = current->getSiguiente();
  }
  return count;
}

#endif // U04_COLAS_COLA_COLA_PRIORIDADGuo