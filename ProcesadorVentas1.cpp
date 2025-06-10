#include "ProcesadorVentas.h"
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <functional>
#include <map>
#include <iomanip>
#include <limits>

// Constructor: Inicializa las estructuras de datos
ProcesadorVentas::ProcesadorVentas()
{
    ventasPorId = new HashMapList<int, Venta>(1000);
    ventasPorMonto = new ArbolBinarioAVL<Venta>();
    pilaEliminaciones = new Pila<Venta>();
    redLogistica = new Grafo<std::string>();
    montoPorProductoPais = new HashMapList<std::string, HashMapList<std::string, double> *>(100);
    promedioPorCategoriaPais = new HashMapList<std::string, HashMapList<std::string, double> *>(100);
    medioEnvioPorPais = new HashMapList<std::string, std::string>(100);
    medioEnvioPorCategoria = new HashMapList<std::string, std::string>(100);
    estadoEnvioPorPais = new HashMapList<std::string, std::string>(100);
    topCiudadesPorPais = new HashMapList<std::string, ColaPrioridad<std::pair<std::string, double>> *>(100);
    diaMayorVentas = "";
    productoMasVendido = "";
    productoMenosVendido = "";

    cargarCSV();
}

// Destructor: Libera la memoria de las estructuras
ProcesadorVentas::~ProcesadorVentas()
{
    delete ventasPorId;
    delete ventasPorMonto;
    delete pilaEliminaciones;
    delete redLogistica;
    delete montoPorProductoPais;
    delete promedioPorCategoriaPais;
    delete medioEnvioPorPais;
    delete medioEnvioPorCategoria;
    delete estadoEnvioPorPais;
    delete topCiudadesPorPais;
}

// Cargar CSV: Lee el archivo y llena las estructuras de datos
void ProcesadorVentas::cargarCSV()
{
    Metricas metricas("Cargar CSV", "HashMapList, ArbolBinarioAVL, Grafo");
    metricas.iniciar();

    std::ifstream file("ventas_sudamerica.csv");
    if (!file.is_open())
    {
        std::cout << "Error: No se pudo abrir el archivo ventas_sudamerica.csv" << std::endl;
        metricas.finalizar();
        metricas.mostrar();
        return;
    }

    std::string linea;
    std::getline(file, linea); // Saltar encabezado

    int lineaNum = 1;
    while (std::getline(file, linea))
    {
        lineaNum++;
        std::stringstream ss(linea);
        std::string campo;
        Venta venta;

        try
        {
            auto limpiar = [](std::string &s)
            {
                s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
                                                { return !std::isspace(ch); }));
                s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
                                     { return !std::isspace(ch); })
                            .base(),
                        s.end());
            };

            std::getline(ss, campo, ',');
            limpiar(campo);
            venta.id_venta = std::stoi(campo);

            std::getline(ss, venta.fecha, ',');
            std::getline(ss, venta.pais, ',');
            std::getline(ss, venta.ciudad, ',');
            std::getline(ss, venta.cliente, ',');
            std::getline(ss, venta.producto, ',');
            std::getline(ss, venta.categoria, ',');

            std::getline(ss, campo, ',');
            limpiar(campo);
            venta.cantidad = std::stoi(campo);

            std::getline(ss, campo, ',');
            limpiar(campo);
            venta.precio_unitario = std::stod(campo);

            std::getline(ss, campo, ',');
            limpiar(campo);
            venta.monto_total = std::stod(campo);

            std::getline(ss, venta.medio_envio, ',');
            std::getline(ss, venta.estado_envio, ',');

            // Insertar en estructuras
            ventasPorId->put(venta.id_venta, venta);
            ventasPorMonto->put(venta);

            try
            {
                if (!venta.ciudad.empty() && !venta.pais.empty())
                {
                    redLogistica->agregarNodo(venta.ciudad);
                    redLogistica->agregarNodo(venta.pais);
                    redLogistica->agregarArista(venta.ciudad, venta.pais);
                }
            }
            catch (const std::exception &e)
            {
                std::cout << "Advertencia al crear grafo (línea " << lineaNum << "): " << e.what() << std::endl;
            }

            metricas.incrementarCondicional();
        }
        catch (const std::exception &e)
        {
            std::cout << "❌ Error al procesar línea " << lineaNum << ": " << linea << "\n   Motivo: " << e.what() << std::endl;
            continue;
        }
    }

    file.close();
    procesarResultados();

    metricas.finalizar();
    metricas.mostrar();
}

// Procesar Resultados: Calcula los resultados principales
void ProcesadorVentas::procesarResultados()
{
    Metricas metricas("Procesar Resultados", "HashMapList, ColaPrioridad, ArbolBinarioAVL");
    metricas.iniciar();

    // Limpiar estructuras previas
    delete montoPorProductoPais;
    delete promedioPorCategoriaPais;
    delete medioEnvioPorPais;
    delete medioEnvioPorCategoria;
    delete estadoEnvioPorPais;
    delete topCiudadesPorPais;
    montoPorProductoPais = new HashMapList<std::string, HashMapList<std::string, double> *>(100);
    promedioPorCategoriaPais = new HashMapList<std::string, HashMapList<std::string, double> *>(100);
    medioEnvioPorPais = new HashMapList<std::string, std::string>(100);
    medioEnvioPorCategoria = new HashMapList<std::string, std::string>(100);
    estadoEnvioPorPais = new HashMapList<std::string, std::string>(100);
    topCiudadesPorPais = new HashMapList<std::string, ColaPrioridad<std::pair<std::string, double>> *>(100);
    diaMayorVentas = "";
    productoMasVendido = "";
    productoMenosVendido = "";

    // Estructuras temporales para cálculos
    std::map<std::string, std::map<std::string, double>> montoProductoPais;                     // Producto -> País -> Monto
    std::map<std::string, std::map<std::string, std::pair<double, int>>> promedioCategoriaPais; // Categoría -> País -> <Suma, Conteo>
    std::map<std::string, std::map<std::string, int>> medioEnvioPais;                           // Medio -> País -> Conteo
    std::map<std::string, std::map<std::string, int>> medioEnvioCategoria;                      // Medio -> Categoría -> Conteo
    std::map<std::string, std::map<std::string, int>> estadoEnvioPais;                          // Estado -> País -> Conteo
    std::map<std::string, std::map<std::string, double>> montoCiudadPais;                       // Ciudad -> País -> Monto
    std::map<std::string, double> montoPorDia;                                                  // Día -> Monto
    std::map<std::string, int> cantidadPorProducto;                                             // Producto -> Cantidad

    // Iterar sobre todas las ventas
    ventasPorId->iterate([&](int id, Venta &venta)
                         {
        // Monto por producto y país
        montoProductoPais[venta.producto][venta.pais] += venta.monto_total;

        // Promedio por categoría y país
        promedioCategoriaPais[venta.categoria][venta.pais].first += venta.monto_total;
        promedioCategoriaPais[venta.categoria][venta.pais].second++;

        // Medio de envío por país
        medioEnvioPais[venta.medio_envio][venta.pais]++;

        // Medio de envío por categoría
        medioEnvioCategoria[venta.medio_envio][venta.categoria]++;

        // Estado de envío por país
        estadoEnvioPais[venta.estado_envio][venta.pais]++;

        // Top ciudades por país
        montoCiudadPais[venta.ciudad][venta.pais] += venta.monto_total;

        // Día con mayor ventas
        montoPorDia[venta.fecha] += venta.monto_total;

        // Producto más/menos vendido (en unidades)
        cantidadPorProducto[venta.producto] += venta.cantidad;

        metricas.incrementarCondicional(); });

    // Llenar estructuras finales
    // Monto por producto y país
    for (const auto &prod : montoProductoPais)
    {
        auto *paisMap = new HashMapList<std::string, double>(10);
        for (const auto &pais : prod.second)
        {
            paisMap->put(pais.first, pais.second);
        }
        montoPorProductoPais->put(prod.first, paisMap);
    }

    // Promedio por categoría y país
    for (const auto &cat : promedioCategoriaPais)
    {
        auto *paisMap = new HashMapList<std::string, double>(10);
        for (const auto &pais : cat.second)
        {
            double promedio = pais.second.second > 0 ? pais.second.first / pais.second.second : 0.0;
            paisMap->put(pais.first, promedio);
        }
        promedioPorCategoriaPais->put(cat.first, paisMap);
    }

    // Medio de envío por país
    for (const auto &medio : medioEnvioPais)
    {
        for (const auto &pais : medio.second)
        {
            std::string current;
            try
            {
                current = medioEnvioPorPais->get(pais.first);
            }
            catch (...)
            {
                current = "";
            }
            for (const auto &medio : medioEnvioPais)
            {
                for (const auto &pais : medio.second)
                {
                    std::string current;
                    int cantidadActual = 0;
                    try
                    {
                        current = medioEnvioPorPais->get(pais.first);
                        cantidadActual = medioEnvioPais[current][pais.first];
                    }
                    catch (...)
                    {
                        current = "";
                    }

                    if (current.empty() || pais.second > cantidadActual)
                    {
                        medioEnvioPorPais->put(pais.first, medio.first);
                    }
                }
            }
        }
    }

    // Medio de envío por categoría
    for (const auto &medio : medioEnvioCategoria)
    {
        for (const auto &cat : medio.second)
        {
            std::string current;
            try
            {
                current = medioEnvioPorCategoria->get(cat.first);
            }
            catch (...)
            {
                current = "";
            }
            if (current.empty() || medio.second.at(cat.first) > medioEnvioCategoria[current][cat.first])
            {
                medioEnvioPorCategoria->put(cat.first, medio.first);
            }
        }
    }

    // Estado de envío por país
    for (const auto &estado : estadoEnvioPais)
    {
        for (const auto &pais : estado.second)
        {
            std::string current;
            try
            {
                current = estadoEnvioPorPais->get(pais.first);
            }
            catch (...)
            {
                current = "";
            }
            if (current.empty() || estado.second.at(pais.first) > estadoEnvioPais[current][pais.first])
            {
                estadoEnvioPorPais->put(pais.first, estado.first);
            }
        }
    }

    // Top 5 ciudades por país
    for (const auto &ciudad : montoCiudadPais)
    {
        const std::string &nombreCiudad = ciudad.first;
        for (const auto &parPaisMonto : ciudad.second)
        {
            const std::string &pais = parPaisMonto.first;
            double monto = parPaisMonto.second;

            // 1) Intenta obtener la cola; si no existe, catch y creas una nueva:
            ColaPrioridad<std::pair<std::string, double>> *cola = nullptr;
            try
            {
                cola = topCiudadesPorPais->get(pais);
            }
            catch (int)
            {
                // no estaba, la creamos:
                cola = new ColaPrioridad<std::pair<std::string, double>>();
                topCiudadesPorPais->put(pais, cola);
            }

            // 2) Encolamos con prioridad (usa encolarPrioridad si quieres que el monto
            //    afecte el orden interno). Aquí ejemplifico con monto como “prioridad”:
            cola->encolarPrioridad({nombreCiudad, monto}, static_cast<int>(-monto));

            // 3) Limitar a 5 elementos
            while (cola->size() > 5)
            {
                cola->desencolar();
            }
        }
    }

    // Día con mayor ventas
    double maxMonto = 0.0;
    for (const auto &dia : montoPorDia)
    {
        if (dia.second > maxMonto)
        {
            maxMonto = dia.second;
            diaMayorVentas = dia.first;
        }
    }

    // Producto más y menos vendido
    int maxCantidad = 0, minCantidad = std::numeric_limits<int>::max();
    for (const auto &prod : cantidadPorProducto)
    {
        if (prod.second > maxCantidad)
        {
            maxCantidad = prod.second;
            productoMasVendido = prod.first;
        }
        if (prod.second < minCantidad && prod.second > 0)
        { // Evitar productos con 0 ventas
            minCantidad = prod.second;
            productoMenosVendido = prod.first;
        }
    }

    metricas.finalizar();
    metricas.mostrar();
}

// Agregar Venta: Guía al usuario para ingresar una nueva venta
void ProcesadorVentas::agregarVenta()
{
    Metricas metricas("Agregar Venta", "HashMapList, ArbolBinarioAVL");
    metricas.iniciar();

    try
    {
        Venta venta;

        std::cout << "Ingrese ID de la venta: ";
        if (!(std::cin >> venta.id_venta && venta.id_venta >= 0)) {
            throw std::invalid_argument("ID invalido. Debe ser un numero entero.");
        }

        // Validar si el ID ya existe
        try {
            ventasPorId->get(venta.id_venta); // Si no lanza, ya existe
            std::cout << "Ya existe una venta con el ID " << venta.id_venta << ".\n";
            metricas.finalizar();
            metricas.mostrar();
            return;
        } catch (...) {
            // No existe el ID, se puede continuar
        }

        std::cin.ignore();

        std::cout << "Ingrese fecha (AAAA-MM-DD): ";
        std::getline(std::cin, venta.fecha);
        std::cout << "Ingrese pais: ";
        std::getline(std::cin, venta.pais);
        std::cout << "Ingrese ciudad: ";
        std::getline(std::cin, venta.ciudad);
        std::cout << "Ingrese cliente: ";
        std::getline(std::cin, venta.cliente);
        std::cout << "Ingrese producto: ";
        std::getline(std::cin, venta.producto);
        std::cout << "Ingrese categoria: ";
        std::getline(std::cin, venta.categoria);

        std::cout << "Ingrese cantidad: ";
        if (!(std::cin >> venta.cantidad) || venta.cantidad <= 0) {
            throw std::invalid_argument("Cantidad invalida. Debe ser un entero positivo.");
        }

        std::cout << "Ingrese precio unitario: ";
        if (!(std::cin >> venta.precio_unitario) || venta.precio_unitario <= 0) {
            throw std::invalid_argument("Precio invalido. Debe ser un numero positivo.");
        }

        venta.monto_total = venta.cantidad * venta.precio_unitario;
        std::cin.ignore();
        std::cout << "Ingrese medio de envio: ";
        std::getline(std::cin, venta.medio_envio);
        std::cout << "Ingrese estado de envio: ";
        std::getline(std::cin, venta.estado_envio);

        // Insertar en estructuras
        ventasPorId->put(venta.id_venta, venta);
        ventasPorMonto->put(venta);

        // Red logística
        if (!venta.ciudad.empty() && !venta.pais.empty())
        {
            try {
                redLogistica->agregarNodo(venta.ciudad);
                redLogistica->agregarNodo(venta.pais);
                redLogistica->agregarArista(venta.ciudad, venta.pais);
            } catch (const std::exception &e) {
                std::cout << "⚠️ Advertencia al agregar al grafo logistico: " << e.what() << "\n";
            }
        }

        reprocesarResultados();
        std::cout << "✅ Venta agregada correctamente.\n";
    }
    catch (const std::invalid_argument &e)
    {
        std::cout << "❌ Error de entrada: " << e.what() << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    catch (...)
    {
        std::cout << "❌ Error inesperado al agregar la venta.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    metricas.finalizar();
    metricas.mostrar();
}



void ProcesadorVentas::eliminarVentaPorID(int id)
{
    Metricas metricas("Eliminar Venta por ID", "HashMapList, ArbolBinarioAVL, Pila");
    metricas.iniciar();

    bool encontrado = false;
    Venta venta;

    ventasPorId->iterate([&](int actualID, Venta &v)
                         {
        if (actualID == id && !encontrado) {
            venta = v;
            encontrado = true;
        } });

    if (!encontrado)
    {
        std::cout << "Error: Venta con ID " << id << " no encontrada.\n";
        metricas.incrementarCondicional();
        metricas.finalizar();
        metricas.mostrar();
        return;
    }

    // Si la encontramos
    std::cout << "Venta encontrada. Procediendo a eliminar...\n";
    venta.imprimir();
    pilaEliminaciones->push(venta);

    try
    {
        ventasPorId->remove(id);
    }
    catch (...)
    {
        std::cout << "No se pudo eliminar del HashMap\n";
    }

    try
    {
        ventasPorMonto->remove(venta);
    }
    catch (...)
    {
        std::cout << "❌ No se pudo eliminar del árbol AVL\n";
    }

    reprocesarResultados();
    std::cout << "Venta con ID " << id << " eliminada correctamente.\n";

    metricas.finalizar();
    metricas.mostrar();
}

// Eliminar Ventas por País/Ciudad: Filtra y elimina ventas
void ProcesadorVentas::eliminarVentasPorPaisCiudad(std::string &pais, std::string &ciudad)
{
    Metricas metricas("Eliminar Ventas por Pais/Ciudad", "HashMapList, ArbolBinarioAVL, Pila");
    metricas.iniciar();

    // Normalizamos entrada
    std::transform(pais.begin(), pais.end(), pais.begin(), ::tolower);
    std::transform(ciudad.begin(), ciudad.end(), ciudad.begin(), ::tolower);

    // Validación de entrada
    auto esValido = [](const std::string &str)
    {
        return std::all_of(str.begin(), str.end(), [](char c)
                           { return std::isalpha(c) || std::isspace(c); });
    };

    if (!esValido(pais) || !esValido(ciudad))
    {
        std::cout << "❌ Entrada invalida. Solo se permiten letras y espacios.\n";
        return;
    }

    std::cout << "[DEBUG] Filtro -> Pais: " << pais << ", Ciudad: " << ciudad << std::endl;
    std::cout << "Ventas encontradas:\n";

    std::vector<int> idsAEliminar;
    ventasPorId->iterate([&](int id, Venta &venta)
                         {
        std::string ventaPais = venta.pais, ventaCiudad = venta.ciudad;
        std::transform(ventaPais.begin(), ventaPais.end(), ventaPais.begin(), ::tolower);
        std::transform(ventaCiudad.begin(), ventaCiudad.end(), ventaCiudad.begin(), ::tolower);

        if ((pais.empty() || ventaPais == pais) && (ciudad.empty() || ventaCiudad == ciudad)) {
            std::cout << "ID: " << id << " - " << venta.pais << " / " << venta.ciudad << std::endl;
            idsAEliminar.push_back(id);
            metricas.incrementarCondicional();
        } });

    if (idsAEliminar.empty())
    {
        std::cout << "❌ No se encontraron ventas con esos filtros. Cancelando eliminación.\n";
        metricas.incrementarCondicional();
        metricas.finalizar();
        metricas.mostrar();
        return;
    }

    std::cout << "[DEBUG] Ventas encontradas: " << idsAEliminar.size() << " registros.\n";
    std::cout << "¿Confirmar eliminacion? (s/n): ";
    char confirm;
    std::cin >> confirm;
    std::cin.ignore();

    if (confirm != 's' && confirm != 'S')
    {
        std::cout << "[DEBUG] Usuario cancelo la eliminación.\n";
        metricas.finalizar();
        metricas.mostrar();
        return;
    }

    std::cout << "[DEBUG] Usuario confirmo. Cargando ventas a eliminar...\n";

    std::vector<Venta> ventasAEliminar;
    ventasPorId->iterate([&](int idActual, Venta &venta)
                         {
    if (std::find(idsAEliminar.begin(), idsAEliminar.end(), idActual) != idsAEliminar.end()) {
        ventasAEliminar.push_back(venta);
        std::cout << "[DEBUG] Cargada venta ID: " << idActual << "\n";
    } });

    std::cout << "[DEBUG] Ventas cargadas. Procediendo a eliminar...\n";

    // 2. Eliminar ventas
    for (const Venta &venta : ventasAEliminar)
    {
        try
        {
            std::cout << "[DEBUG] Eliminando venta ID: " << venta.id_venta << "\n";
            pilaEliminaciones->push(venta);
            ventasPorId->remove(venta.id_venta);
            ventasPorMonto->remove(venta);
        }
        catch (...)
        {
            std::cout << "❌ Error eliminando venta con ID: " << venta.id_venta << "\n";
        }
    }

    std::cout << "[DEBUG] Ventas eliminadas. Reprocesando resultados...\n";

    reprocesarResultados();

    std::cout << "✅ Ventas eliminadas correctamente.\n";

    metricas.finalizar();
    metricas.mostrar();
}

// Modificar Venta: Actualiza una venta existente
void ProcesadorVentas::modificarVenta(int id)
{
    Metricas metricas("Modificar Venta", "HashMapList, ArbolBinarioAVL");
    metricas.iniciar();

    bool encontrado = false;
    Venta ventaOriginal;

    // Buscar la venta con ID dado usando iterate (como eliminarVentaPorID)
    ventasPorId->iterate([&](int actualID, Venta &v)
                         {
        if (actualID == id && !encontrado) {
            ventaOriginal = v;
            encontrado = true;
        } });

    if (!encontrado)
    {
        std::cout << "❌ Error: Venta con ID " << id << " no encontrada.\n";
        metricas.incrementarCondicional();
        metricas.finalizar();
        metricas.mostrar();
        return;
    }

    Venta venta = ventaOriginal; // Copia modificable

    std::cout << "Venta actual:\n";
    venta.imprimir();

    std::string input;
    std::cout << "Ingrese nueva fecha (dejar vacio para no cambiar): ";
    std::getline(std::cin, input);
    if (!input.empty())
        venta.fecha = input;

    std::cout << "Ingrese nuevo pais (dejar vacio para no cambiar): ";
    std::getline(std::cin, input);
    if (!input.empty())
        venta.pais = input;

    std::cout << "Ingrese nueva ciudad (dejar vacio para no cambiar): ";
    std::getline(std::cin, input);
    if (!input.empty())
        venta.ciudad = input;

    std::cout << "Ingrese nuevo cliente (dejar vacio para no cambiar): ";
    std::getline(std::cin, input);
    if (!input.empty())
        venta.cliente = input;

    std::cout << "Ingrese nuevo producto (dejar vacio para no cambiar): ";
    std::getline(std::cin, input);
    if (!input.empty())
        venta.producto = input;

    std::cout << "Ingrese nueva categoria (dejar vacio para no cambiar): ";
    std::getline(std::cin, input);
    if (!input.empty())
        venta.categoria = input;

    std::cout << "Ingrese nueva cantidad (0 para no cambiar): ";
    int cantidad;
    std::cin >> cantidad;
    if (cantidad > 0)
        venta.cantidad = cantidad;

    std::cout << "Ingrese nuevo precio unitario (0 para no cambiar): ";
    double precio;
    std::cin >> precio;
    if (precio > 0)
        venta.precio_unitario = precio;

    std::cin.ignore();
    std::cout << "Ingrese nuevo medio de envio (dejar vacio para no cambiar): ";
    std::getline(std::cin, input);
    if (!input.empty())
        venta.medio_envio = input;

    std::cout << "Ingrese nuevo estado de envio (dejar vacio para no cambiar): ";
    std::getline(std::cin, input);
    if (!input.empty())
        venta.estado_envio = input;

    // Recalcular monto total
    venta.monto_total = venta.cantidad * venta.precio_unitario;

    // Eliminar venta original de ambas estructuras
    try
    {
        ventasPorId->remove(id);
    }
    catch (...)
    {
        std::cout << "No se pudo eliminar del HashMap\n";
    }

    try
    {
        ventasPorMonto->remove(ventaOriginal);
    }
    catch (...)
    {
        std::cout << "No se pudo eliminar del arbol AVL\n";
    }

    // Insertar la venta modificada
    ventasPorId->put(id, venta);
    ventasPorMonto->put(venta);

    reprocesarResultados();
    std::cout << "Venta modificada correctamente.\n";

    metricas.finalizar();
    metricas.mostrar();
}

// Listar Ventas por Ciudad: Muestra ventas de una ciudad específica
void ProcesadorVentas::listarVentasPorCiudad(const std::string &ciudadInput)
{
    Metricas metricas("Listar Ventas por Ciudad", "HashMapList");
    metricas.iniciar();

    bool encontradas = false;

    std::string ciudadBuscada = ciudadInput;
    std::transform(ciudadBuscada.begin(), ciudadBuscada.end(), ciudadBuscada.begin(), ::tolower); // 🔽 Convertimos a minúsculas

    ventasPorId->iterate([&](int id, Venta &venta)
                         {
        std::string ciudadVenta = venta.ciudad;
        std::transform(ciudadVenta.begin(), ciudadVenta.end(), ciudadVenta.begin(), ::tolower);

        if (ciudadVenta == ciudadBuscada) {
            venta.imprimir();
            encontradas = true;
            metricas.incrementarCondicional();
        } });

    if (!encontradas)
    {
        std::cout << "❌ No se encontraron ventas en la ciudad \"" << ciudadInput << "\".\n";
        metricas.incrementarCondicional();
    }

    metricas.finalizar();
    metricas.mostrar();
}


int convertirFechaAEntero(std::string fecha)
{
    // Reemplazar guiones por barras por si el usuario usa "-"
    std::replace(fecha.begin(), fecha.end(), '-', '/');

    std::stringstream ss(fecha);
    std::string dia, mes, anio;
    std::getline(ss, dia, '/');
    std::getline(ss, mes, '/');
    std::getline(ss, anio, '/');

    // Validar existencia de todos los componentes
    if (dia.empty() || mes.empty() || anio.empty()) {
        throw std::invalid_argument("Formato de fecha invalido. Se esperaba DD/MM/YYYY.");
    }

    // Asegurar formato de dos digitos para dia y mes
    if (dia.size() == 1) dia = "0" + dia;
    if (mes.size() == 1) mes = "0" + mes;

    return std::stoi(anio + mes + dia); // Formato AAAAMMDD
}


void ProcesadorVentas::listarVentasPorRangoFechas(const std::string &pais, const std::string &fechaInicio, const std::string &fechaFin)
{
    Metricas metricas("Listar Ventas por Rango de Fechas", "HashMapList");
    metricas.iniciar();

    bool encontradas = false;

    try
    {
        int inicio = convertirFechaAEntero(fechaInicio);
        int fin = convertirFechaAEntero(fechaFin);

        std::string pais_normalizado = pais;
        std::transform(pais_normalizado.begin(), pais_normalizado.end(), pais_normalizado.begin(), ::tolower);

        ventasPorId->iterate([&](int id, Venta &venta)
        {
            std::string paisVenta = venta.pais;
            std::transform(paisVenta.begin(), paisVenta.end(), paisVenta.begin(), ::tolower);

            int fechaVenta = convertirFechaAEntero(venta.fecha);

            if ((pais.empty() || pais_normalizado == paisVenta) &&
                fechaVenta >= inicio && fechaVenta <= fin)
            {
                venta.imprimir();
                encontradas = true;
                metricas.incrementarCondicional();
            }
        });

        if (!encontradas)
        {
            std::cout << "No se encontraron ventas en el rango de fechas para el pais especificado." << std::endl;
            metricas.incrementarCondicional();
        }
    }
    catch (const std::invalid_argument &e)
    {
        std::cout << "Error: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "Error inesperado al procesar las fechas ingresadas.\n";
    }

    metricas.finalizar();
    metricas.mostrar();
}



// Comparar Países: Compara monto total, productos más vendidos y medio de envío
void ProcesadorVentas::compararPaises(const std::string &input1, const std::string &input2)
{
    Metricas metricas("Comparar Paises", "HashMapList");
    metricas.iniciar();

    try
    {
        std::string pais1 = input1;
        std::string pais2 = input2;

        // Validar que los nombres no esten vacios
        if (pais1.empty() || pais2.empty()) {
            throw std::invalid_argument("Ambos paises deben tener nombres validos.");
        }

        // Verificar si son iguales
        if (pais1 == pais2) {
            std::cout << "No tiene sentido comparar el mismo pais contra si mismo: " << pais1 << ".\n";
            metricas.finalizar();
            metricas.mostrar();
            return;
        }

        double montoTotal1 = 0, montoTotal2 = 0;
        std::string productoMax1, productoMax2;
        double maxMonto1 = 0, maxMonto2 = 0;
        bool datos1 = false, datos2 = false;

        // Recorrer ventas
        ventasPorId->iterate([&](int id, Venta &venta)
        {
            if (venta.pais == pais1) {
                montoTotal1 += venta.monto_total;
                if (venta.monto_total > maxMonto1) {
                    productoMax1 = venta.producto;
                    maxMonto1 = venta.monto_total;
                }
                datos1 = true;
                metricas.incrementarCondicional();
            }
            if (venta.pais == pais2) {
                montoTotal2 += venta.monto_total;
                if (venta.monto_total > maxMonto2) {
                    productoMax2 = venta.producto;
                    maxMonto2 = venta.monto_total;
                }
                datos2 = true;
                metricas.incrementarCondicional();
            }
        });

        // Mensajes si no hay datos
        if (!datos1 && !datos2) {
            std::cout << "No se encontraron ventas para ninguno de los dos paises.\n";
            metricas.finalizar();
            metricas.mostrar();
            return;
        }
        if (!datos1) {
            std::cout << "No se encontraron ventas para " << pais1 << ".\n";
        }
        if (!datos2) {
            std::cout << "No se encontraron ventas para " << pais2 << ".\n";
        }

        std::cout << "Comparacion entre " << pais1 << " y " << pais2 << ":\n";
        std::cout << "Monto total " << pais1 << ": $" << montoTotal1 << "\n";
        std::cout << "Monto total " << pais2 << ": $" << montoTotal2 << "\n";
        std::cout << "Producto mas vendido " << pais1 << ": " << (productoMax1.empty() ? "Ninguno" : productoMax1) << "\n";
        std::cout << "Producto mas vendido " << pais2 << ": " << (productoMax2.empty() ? "Ninguno" : productoMax2) << "\n";

        // Medio de envio por pais
        try {
            std::string medio1 = medioEnvioPorPais->get(pais1);
            std::cout << "Medio de envio mas usado " << pais1 << ": " << medio1 << "\n";
        } catch (...) {
            std::cout << "No hay medio de envio para " << pais1 << ".\n";
        }

        try {
            std::string medio2 = medioEnvioPorPais->get(pais2);
            std::cout << "Medio de envio mas usado " << pais2 << ": " << medio2 << "\n";
        } catch (...) {
            std::cout << "No hay medio de envio para " << pais2 << ".\n";
        }
    }
    catch (const std::invalid_argument &e)
    {
        std::cout << "Error: " << e.what() << "\n";
    }
    catch (...)
    {
        std::cout << "Error inesperado al comparar paises.\n";
    }

    metricas.finalizar();
    metricas.mostrar();
}

// Comparar Productos: Compara cantidad y monto total por país
void ProcesadorVentas::compararProductos(const std::string &producto1, const std::string &producto2)
{
    Metricas metricas("Comparar Productos", "HashMapList");
    metricas.iniciar();

    std::string p1 = producto1;
    std::string p2 = producto2;

    // Estandarización opcional por si se usa en otros lugares
    std::transform(p1.begin(), p1.end(), p1.begin(), ::tolower);
    std::transform(p2.begin(), p2.end(), p2.begin(), ::tolower);

    HashMapList<std::string, double> montoProducto1(100), montoProducto2(100);
    HashMapList<std::string, int> cantidadProducto1(100), cantidadProducto2(100);

    ventasPorId->iterate([&](int id, Venta &venta)
                         {
        std::string prod = venta.producto;
        std::transform(prod.begin(), prod.end(), prod.begin(), ::tolower);

        if (prod == p1) {
            try {
                montoProducto1.put(venta.pais, montoProducto1.get(venta.pais) + venta.monto_total);
            } catch (...) {
                montoProducto1.put(venta.pais, venta.monto_total);
            }

            try {
                cantidadProducto1.put(venta.pais, cantidadProducto1.get(venta.pais) + venta.cantidad);
            } catch (...) {
                cantidadProducto1.put(venta.pais, venta.cantidad);
            }

            metricas.incrementarCondicional();
        }
        else if (prod == p2) {
            try {
                montoProducto2.put(venta.pais, montoProducto2.get(venta.pais) + venta.monto_total);
            } catch (...) {
                montoProducto2.put(venta.pais, venta.monto_total);
            }

            try {
                cantidadProducto2.put(venta.pais, cantidadProducto2.get(venta.pais) + venta.cantidad);
            } catch (...) {
                cantidadProducto2.put(venta.pais, venta.cantidad);
            }

            metricas.incrementarCondicional();
        } });

    std::cout << "Comparacion entre " << producto1 << " y " << producto2 << ":\n";

    montoProducto1.iterate([&](std::string pais, double monto)
                           {
        int cantidad = cantidadProducto1.get(pais);
        std::cout << pais << " - " << producto1 << ": Cantidad: " << cantidad << ", Monto: $" << monto << "\n"; });

    montoProducto2.iterate([&](std::string pais, double monto)
                           {
        int cantidad = cantidadProducto2.get(pais);
        std::cout << pais << " - " << producto2 << ": Cantidad: " << cantidad << ", Monto: $" << monto << "\n"; });

    metricas.finalizar();
    metricas.mostrar();
}

// Productos por Monto Promedio: Busca productos por encima o debajo de un umbral
void ProcesadorVentas::productosPorMontoPromedio(double umbral, bool porEncima)
{
    Metricas metricas("Productos por Monto Promedio", "HashMapList");
    metricas.iniciar();

    HashMapList<std::string, HashMapList<std::string, std::pair<double, int>> *> promedios(100);
    ventasPorId->iterate([&](int id, Venta &venta)
                         {
        try {
            auto* datos = promedios.get(venta.pais);
            try {
                auto par = datos->get(venta.producto);
                datos->put(venta.producto, {par.first + venta.monto_total, par.second + venta.cantidad});
            } catch (...) {
                datos->put(venta.producto, {venta.monto_total, venta.cantidad});
            }
        } catch (...) {
            auto* datos = new HashMapList<std::string, std::pair<double, int>>(100);
            datos->put(venta.producto, {venta.monto_total, venta.cantidad});
            promedios.put(venta.pais, datos);
        }
        metricas.incrementarCondicional(); });

    promedios.iterate([&](std::string pais, HashMapList<std::string, std::pair<double, int>> *&datos)
                      {
        std::cout << "Pais: " << pais << std::endl;
        datos->iterate([&](std::string producto, std::pair<double, int> par) {
            double promedio = par.first / par.second;
            if ((porEncima && promedio > umbral) || (!porEncima && promedio < umbral)) {
                std::cout << "  Producto: " << producto << ", Promedio: $" << promedio << std::endl;
                metricas.incrementarCondicional();
            }
        }); });

    metricas.finalizar();
    metricas.mostrar();
}

// Analizar Red Logística: Muestra conexiones entre ciudades
void ProcesadorVentas::analizarRedLogistica()
{
    Metricas metricas("Analizar Red Logística", "Grafo");
    metricas.iniciar();

    std::string origen, destino;
    std::cout << "Ingrese el primer nodo (ciudad o país): ";
    std::getline(std::cin, origen);
    std::cout << "Ingrese el segundo nodo (ciudad o país): ";
    std::getline(std::cin, destino);

    std::cout << "¿Estan conectados " << origen << " y " << destino << "? ";
    if (redLogistica->estanConectados(origen, destino))
    {
        std::cout << "Si, estan conectados.\n";
        metricas.incrementarCondicional();
    }
    else
    {
        std::cout << "No, no estan conectados.\n";
        metricas.incrementarCondicional();
    }

    metricas.finalizar();
    metricas.mostrar();
}

// Mostrar Top 5 Ciudades por País
void ProcesadorVentas::mostrarTop5CiudadesPorPais()
{
    Metricas metricas("Top 5 Ciudades por Pais", "HashMapList, ColaPrioridad");
    metricas.iniciar();

    std::cout << "\nTop 5 ciudades por pais (monto total de ventas):\n";
    topCiudadesPorPais->iterate([&](const std::string &pais, ColaPrioridad<std::pair<std::string, double>> *&cola)
                                {
        std::cout << "  " << pais << ":\n";
        ColaPrioridad<std::pair<std::string, double>> temp(*cola); // Copia para no modificar la original

        int count = 0;
        while (!temp.esVacia() && count < 5) {
            auto par = temp.desencolar();
            std::cout << "    " << par.first << ": $" << std::fixed << std::setprecision(2) << par.second << "\n";
            metricas.incrementarCondicional();
            count++;
        } });

    metricas.finalizar();
    metricas.mostrar();
}

// Mostrar Monto Total por Producto y País
void ProcesadorVentas::mostrarMontoTotalPorProductoPais()
{
    Metricas metricas("Monto Total por Producto y Pais", "HashMapList");
    metricas.iniciar();

    std::cout << "\nMonto total por producto en cada pais:\n";
    montoPorProductoPais->iterate([&](const std::string &producto, HashMapList<std::string, double> *&paisMap)
                                  {
        std::cout << "  " << producto << ":\n";
        paisMap->iterate([&](const std::string& pais, double& monto) {
            std::cout << "    " << pais << ": $" << std::fixed << std::setprecision(2) << monto << "\n";
        }); });

    metricas.finalizar();
    metricas.mostrar();
}

// Mostrar Promedio de Ventas por Categoría y País
void ProcesadorVentas::mostrarPromedioVentasPorCategoriaPais()
{
    Metricas metricas("Promedio de Ventas por Categoria y Pais", "HashMapList");
    metricas.iniciar();

    std::cout << "\nPromedio de ventas por categoria en cada pais:\n";
    promedioPorCategoriaPais->iterate([&](const std::string &categoria, HashMapList<std::string, double> *&paisMap)
                                      {
        std::cout << "  " << categoria << ":\n";
        paisMap->iterate([&](const std::string& pais, double& promedio) {
            std::cout << "    " << pais << ": $" << std::fixed << std::setprecision(2) << promedio << "\n";
        }); });

    metricas.finalizar();
    metricas.mostrar();
}

// Mostrar Medio de Envío Más Utilizado por País
void ProcesadorVentas::mostrarMedioEnvioMasUtilizadoPorPais()
{
    Metricas metricas("Medio de Envio Mas Utilizado por Pais", "HashMapList");
    metricas.iniciar();

    std::cout << "\nMedio de envio mas utilizado por pais:\n";
    medioEnvioPorPais->iterate([&](const std::string &pais, std::string &medio)
                               { std::cout << "  " << pais << ": " << medio << "\n"; });

    metricas.finalizar();
    metricas.mostrar();
}

// Mostrar Medio de Envío Más Utilizado por Categoría
void ProcesadorVentas::mostrarMedioEnvioMasUtilizadoPorCategoria()
{
    Metricas metricas("Medio de Envio Mas Utilizado por Categoria", "HashMapList");
    metricas.iniciar();

    std::cout << "\nMedio de envio mas utilizado por categoria:\n";
    medioEnvioPorCategoria->iterate([&](const std::string &categoria, std::string &medio)
                                    { std::cout << "  " << categoria << ": " << medio << "\n"; });

    metricas.finalizar();
    metricas.mostrar();
}

// Mostrar Día con Mayor Ventas
void ProcesadorVentas::mostrarDiaMayorVentas()
{
    Metricas metricas("Dia con Mayor Ventas", "HashMapList");
    metricas.iniciar();

    std::cout << "\nDia con mayor monto de ventas:\n";
    std::cout << "  " << diaMayorVentas << "\n";

    metricas.finalizar();
    metricas.mostrar();
}

// Mostrar Estado de Envío Más Frecuente por País
void ProcesadorVentas::mostrarEstadoEnvioMasFrecuentePorPais()
{
    Metricas metricas("Estado de Envio Más Frecuente por Pais", "HashMapList");
    metricas.iniciar();

    std::cout << "\nEstado de envio mas frecuente por pais:\n";
    estadoEnvioPorPais->iterate([&](const std::string &pais, std::string &estado)
                                { std::cout << "  " << pais << ": " << estado << "\n"; });

    metricas.finalizar();
    metricas.mostrar();
}

// Mostrar Producto Más Vendido
void ProcesadorVentas::mostrarProductoMasVendido()
{
    Metricas metricas("Producto Mas Vendido", "HashMapList");
    metricas.iniciar();

    std::cout << "\nProducto mas vendido (en unidades):\n";
    std::cout << "  " << productoMasVendido << "\n";

    metricas.finalizar();
    metricas.mostrar();
}

// Mostrar Producto Menos Vendido
void ProcesadorVentas::mostrarProductoMenosVendido()
{
    Metricas metricas("Producto Menos Vendido", "HashMapList");
    metricas.iniciar();

    std::cout << "\nProducto menos vendido (en unidades):\n";
    std::cout << "  " << productoMenosVendido << "\n";

    metricas.finalizar();
    metricas.mostrar();
}

// Mostrar Resultados: Muestra los resultados precalculados
void ProcesadorVentas::mostrarResultados()
{
    Metricas metricas("Mostrar Resultados", "HashMapList, ColaPrioridad");
    metricas.iniciar();

    std::cout << "\nResultados Principales:\n";

    // Top 5 ciudades por país
    std::cout << "\nTop 5 ciudades por pais (monto total de ventas):\n";
    topCiudadesPorPais->iterate([&](const std::string &pais, ColaPrioridad<std::pair<std::string, double>> *&cola)
                                {
        std::cout << "  " << pais << ":\n";
        ColaPrioridad<std::pair<std::string, double>> temp(*cola); // Copia para no modificar la original
        while (!temp.esVacia()) {
            auto par = temp.desencolar();
            std::cout << "    " << par.first << ": $" << std::fixed << std::setprecision(2) << par.second << "\n";
            metricas.incrementarCondicional();
        } });

    // Monto total por producto y país
    std::cout << "\nMonto total por producto en cada pais:\n";
    montoPorProductoPais->iterate([&](const std::string &producto, HashMapList<std::string, double> *&paisMap)
                                  {
        std::cout << "  " << producto << ":\n";
        paisMap->iterate([&](const std::string& pais, double& monto) {
            std::cout << "    " << pais << ": $" << std::fixed << std::setprecision(2) << monto << "\n";
        }); });

    // Promedio por categoría y país
    std::cout << "\nPromedio de ventas por categoria en cada pais:\n";
    promedioPorCategoriaPais->iterate([&](const std::string &categoria, HashMapList<std::string, double> *&paisMap)
                                      {
        std::cout << "  " << categoria << ":\n";
        paisMap->iterate([&](const std::string& pais, double& promedio) {
            std::cout << "    " << pais << ": $" << std::fixed << std::setprecision(2) << promedio << "\n";
        }); });

    // Medio de envío más usado por país
    std::cout << "\nMedio de envio mas utilizado por pais:\n";
    medioEnvioPorPais->iterate([&](const std::string &pais, std::string &medio)
                               { std::cout << "  " << pais << ": " << medio << "\n"; });

    // Medio de envío más usado por categoría
    std::cout << "\nMedio de envio mas utilizado por categoria:\n";
    medioEnvioPorCategoria->iterate([&](const std::string &categoria, std::string &medio)
                                    { std::cout << "  " << categoria << ": " << medio << "\n"; });

    // Día con mayor ventas
    std::cout << "\nDia con mayor monto de ventas:\n";
    std::cout << "  " << diaMayorVentas << "\n";

    // Estado de envío más frecuente por país
    std::cout << "\nEstado de envio mas frecuente por pais:\n";
    estadoEnvioPorPais->iterate([&](const std::string &pais, std::string &estado)
                                { std::cout << "  " << pais << ": " << estado << "\n"; });

    // Producto más y menos vendido
    std::cout << "\nProducto mas vendido (en unidades):\n";
    std::cout << "  " << productoMasVendido << "\n";
    std::cout << "\nProducto menos vendido (en unidades):\n";
    std::cout << "  " << productoMenosVendido << "\n";

    metricas.finalizar();
    metricas.mostrar();
}

// Reprocesar Resultados: Llama a procesarResultados tras modificaciones
void ProcesadorVentas::reprocesarResultados()
{
    procesarResultados();
}
