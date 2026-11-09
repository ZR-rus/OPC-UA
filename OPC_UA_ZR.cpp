
#include <tchar.h>
#include <windows.h>
#include <open62541.h>


// Глобальные переменные
UA_Client* g_client = nullptr;
HWND g_hButtonConnect, g_hEditInfo, g_hEditStatus, g_hEditNumber, g_hEditSIS, g_hEditVoltage,
g_hEditPressure, g_hEditAngle, g_hEditStatusBit[16], g_hEditStatusButton[16], g_hButtonReadCustom;
wchar_t result_dec[512] = { 0 };
wchar_t result_bool[512] = { 0 };
int id_OPC;
static const wchar_t* Button_Visible[4] = { L"машина ",L"вода ",L"сис ",L"авторежим " };
static const wchar_t* Button[16] = { L"автоперезапуск", L" ",L"вода", L"направление", L"СИС", L" ", L"пуск", L"пуск",
 L" ", L" ", L"пуск с автоперезап", L" ", L"автореверс", L"авторежим", L" ",  L" " };
// Прототипы функций
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void ConnectToOpcUaServer(HWND hwnd);
void ReadCustomVariable(HWND hwnd);

// Идентификаторы элементов управления
#define IDC_BUTTON_CONNECT 1001
#define IDC_EDIT_STATUS 1002
#define IDC_BUTTON_READ_CUSTOM 1004

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSEX wcex = {};
    HWND hWnd;
    MSG msg;

    // Регистрация класса окна
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"OpcUa";

    if (!RegisterClassEx(&wcex))
        return 0;

    // Создание окна
    hWnd = CreateWindowEx(0, L"OpcUa", L"OPC клиент",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 320, 500,
        NULL, NULL, hInstance, NULL);

    if (!hWnd)
        return 0;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Цикл сообщений
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
    {
        // Создание элементов управления
        g_hButtonConnect = CreateWindow(L"BUTTON", L"Подключиться к серверу",
            WS_VISIBLE | WS_CHILD, 20, 30, 200, 30, hWnd,
            (HMENU)IDC_BUTTON_CONNECT, NULL, NULL);

        g_hEditInfo = CreateWindow(L"EDIT", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE,
            20, 70, 270, 20, hWnd, (HMENU)IDC_EDIT_STATUS, NULL, NULL);

        HWND g_hStaticNumber = CreateWindow(L"STATIC", L"Номер Valley:",
            WS_VISIBLE | WS_CHILD,
            20, 100, 130, 20, hWnd, NULL, NULL, NULL);

        g_hEditNumber = CreateWindow(L"EDIT", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE,
            170, 100, 50, 20, hWnd, (HMENU)IDC_EDIT_STATUS, NULL, NULL);

        HWND g_hStaticSIS = CreateWindow(L"STATIC", L"СИС (градусы):",
            WS_VISIBLE | WS_CHILD,
            20, 130, 130, 20, hWnd, NULL, NULL, NULL);

        g_hEditSIS = CreateWindow(L"EDIT", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE,
            170, 130, 50, 20, hWnd, (HMENU)IDC_EDIT_STATUS, NULL, NULL);

        HWND g_hStaticVoltage = CreateWindow(L"STATIC", L"Напряжение сети:",
            WS_VISIBLE | WS_CHILD,
            20, 160, 130, 20, hWnd, NULL, NULL, NULL);

        g_hEditVoltage = CreateWindow(L"EDIT", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE,
            170, 160, 50, 20, hWnd, (HMENU)IDC_EDIT_STATUS, NULL, NULL);

        HWND g_hStaticPressure = CreateWindow(L"STATIC", L"Давление воды:",
            WS_VISIBLE | WS_CHILD,
            20, 190, 130, 20, hWnd, NULL, NULL, NULL);

        g_hEditPressure = CreateWindow(L"EDIT", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE,
            170, 190, 50, 20, hWnd, (HMENU)IDC_EDIT_STATUS, NULL, NULL);

        HWND g_hStaticAngle = CreateWindow(L"STATIC", L"Положение:",
            WS_VISIBLE | WS_CHILD,
            20, 220, 130, 20, hWnd, NULL, NULL, NULL);

        g_hEditAngle = CreateWindow(L"EDIT", L"",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE,
            170, 220, 50, 20, hWnd, (HMENU)IDC_EDIT_STATUS, NULL, NULL);

        for (int i = 0; i < 16; i++) {
            HWND g_hStaticButton = CreateWindow(L"STATIC", Button[i],
                WS_VISIBLE | WS_CHILD,
                350, 70 + 20 * i, 140, 20, hWnd, NULL, NULL, NULL);
            //g_hEditStatusButton[i] = CreateWindow(L"EDIT", L"",
            //    WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE,
            //    400, 280 + 20 * i, 80, 20, hWnd, (HMENU)IDC_EDIT_STATUS, NULL, NULL);
            g_hEditStatusBit[i] = CreateWindow(L"EDIT", L"",
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE,
                500, 70 + 20 * i, 20, 20, hWnd, (HMENU)IDC_EDIT_STATUS, NULL, NULL);
        }
        for (int i = 0; i < 4; i++) {
            HWND g_hStaticButton = CreateWindow(L"STATIC", Button_Visible[i],
                WS_VISIBLE | WS_CHILD,
                20, 300 + 30 * i, 80, 20, hWnd, NULL, NULL, NULL);
            g_hEditStatusButton[i] = CreateWindow(L"EDIT", L"",
                WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE,
                110, 300 + 30 * i, 150, 20, hWnd, (HMENU)IDC_EDIT_STATUS, NULL, NULL);
        }

        HWND g_hStaticStatus = CreateWindow(L"STATIC", L"Статус машины:",
            /*WS_VISIBLE |*/ WS_CHILD,
            20, 550, 130, 20, hWnd, NULL, NULL, NULL);

        g_hEditStatus = CreateWindow(L"EDIT", L"",
            /*WS_VISIBLE |*/ WS_CHILD | WS_BORDER | ES_READONLY | ES_MULTILINE,
            170, 550, 50, 20, hWnd, (HMENU)IDC_EDIT_STATUS, NULL, NULL);

        g_hButtonReadCustom = CreateWindow(L"BUTTON", L"Обновить",
            /*WS_VISIBLE |*/ WS_CHILD | WS_DISABLED, 20, 600, 200, 30,
            hWnd, (HMENU)IDC_BUTTON_READ_CUSTOM, NULL, NULL);
    }
    break;

    case WM_TIMER:
        if (wParam == 1) {
            SendMessage(hWnd, WM_COMMAND, (WPARAM)IDC_BUTTON_READ_CUSTOM, (LPARAM)lParam);// отправить сообщение в очередь
        }
        break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case IDC_BUTTON_CONNECT:
            ConnectToOpcUaServer(hWnd);
            SetTimer(hWnd, 1, 5000, NULL);
            break;

        case IDC_BUTTON_READ_CUSTOM:
            for (int i = 0; i < 512; i++) {
                result_bool[i] = 0;
            }

            id_OPC = 20;
            ReadCustomVariable(hWnd);
            uint16_t  result_DEC = 0;
            result_DEC = _wtoi(result_dec);
            result_DEC = result_DEC - 38;
            swprintf_s(result_dec, sizeof(result_dec) / sizeof(wchar_t), L"%u", result_DEC);
            SetWindowText(g_hEditNumber, result_dec);

            id_OPC = 22;
            ReadCustomVariable(hWnd);
            SetWindowText(g_hEditStatus, result_dec);
            result_DEC = 0;
            result_DEC = _wtoi(result_dec);
            for (int i = 0; i < 16; i++) {
                result_bool[i] = (result_DEC & (32768 >> i)) / (32768 >> i) + 48;
                SetWindowText(g_hEditStatusBit[i], &result_bool[i]);
            }
            if (result_bool[3] == 49 && result_bool[6] == 49 && result_bool[7] == 49) SetWindowText(g_hEditStatusButton[0], L"движется назад");
            if (result_bool[3] == 48 && result_bool[6] == 49 && result_bool[7] == 49) SetWindowText(g_hEditStatusButton[0], L"движется вперед");
            if (result_bool[6] == 48 && result_bool[7] == 48) SetWindowText(g_hEditStatusButton[0], L"остановлена");
            if (result_bool[2] == 49) SetWindowText(g_hEditStatusButton[1], L"включена");
            if (result_bool[2] == 48) SetWindowText(g_hEditStatusButton[1], L"отключена");
            if (result_bool[4] == 49) SetWindowText(g_hEditStatusButton[2], L"включен");
            if (result_bool[4] == 48) SetWindowText(g_hEditStatusButton[2], L"отключен");
            if (result_bool[13] == 49) 
               {
                 if (result_bool[12] == 49) SetWindowText(g_hEditStatusButton[3], L"автореверс");
                 if (result_bool[12] == 48) SetWindowText(g_hEditStatusButton[3], L"автостоп");
               }
            if (result_bool[13] == 48) { SetWindowText(g_hEditStatusButton[3], L"деактивирован"); SetWindowText(g_hEditStatusButton[5], L"-"); SetWindowText(g_hEditStatusButton[4], L"-"); }

            id_OPC = 29;
            ReadCustomVariable(hWnd);
            SetWindowText(g_hEditSIS, result_dec);

            id_OPC = 30;
            ReadCustomVariable(hWnd);
            SetWindowText(g_hEditPressure, result_dec);

            id_OPC = 31;
            ReadCustomVariable(hWnd);
            SetWindowText(g_hEditVoltage, result_dec);

            id_OPC = 32;
            ReadCustomVariable(hWnd);
            SetWindowText(g_hEditAngle, result_dec);
            break;
        }
    }
    break;

    case WM_DESTROY:
        // Очистка клиента при закрытии приложения
        if (g_client) {
            UA_Client_disconnect(g_client);
            UA_Client_delete(g_client);
        }
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void ConnectToOpcUaServer(HWND hwnd) {
    // Создаём клиент
    g_client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(g_client));

    // Подключаемся к серверу (измените URL на нужный)
    UA_StatusCode retval = UA_Client_connect(g_client, "opc.tcp://192.168.200.103:4840");

    if (retval == UA_STATUSCODE_GOOD) {
        SetWindowText(g_hEditInfo, L"Подключено успешно");
        EnableWindow(g_hButtonReadCustom, TRUE);
    }
    else {
        wchar_t errorMsg[256];
        swprintf_s(errorMsg, L"Ошибка подключения: %S", UA_StatusCode_name(retval));
        SetWindowText(g_hEditInfo, errorMsg);
    }
}

void ReadCustomVariable(HWND hwnd) {
    if (!g_client) return;

    UA_Variant value;
    UA_Variant_init(&value);  // Инициализация варианта

    // Определяем узел 
    UA_NodeId nodeId;
    nodeId.namespaceIndex = 4;
    nodeId.identifierType = UA_NODEIDTYPE_NUMERIC;
    nodeId.identifier.numeric = id_OPC;

    // Читаем значение атрибута
    UA_StatusCode retval = UA_Client_readValueAttribute(g_client, nodeId, &value);

    if (retval == UA_STATUSCODE_GOOD) {
        // Обрабатываем разные типы данных
        wchar_t result[512] = { 0 };//= L"Значение переменной: ";

        if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_BOOLEAN])) {
            swprintf_s(result + wcslen(result), 512 - wcslen(result), L"%s",
                *(UA_Boolean*)value.data ? L"TRUE" : L"FALSE");
        }
        else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_BYTE])) {
            swprintf_s(result + wcslen(result), 512 - wcslen(result), L"%u", *(UA_Byte*)value.data);
        }
        else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT16])) {
            swprintf_s(result + wcslen(result), 512 - wcslen(result), L"%d", *(UA_Int16*)value.data);
        }
        else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_UINT16])) {
            swprintf_s(result + wcslen(result), 512 - wcslen(result), L"%u", *(UA_UInt16*)value.data);
        }
        else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32])) {
            swprintf_s(result + wcslen(result), 512 - wcslen(result), L"%d", *(UA_Int32*)value.data);
        }
        else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_UINT32])) {
            swprintf_s(result + wcslen(result), 512 - wcslen(result), L"%u", *(UA_UInt32*)value.data);
        }
        else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_FLOAT])) {
            swprintf_s(result + wcslen(result), 512 - wcslen(result), L"%.1f", *(UA_Float*)value.data);
        }
        else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_DOUBLE])) {
            swprintf_s(result + wcslen(result), 512 - wcslen(result), L"%.6f", *(UA_Double*)value.data);
        }
        else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_STRING])) {
            UA_String* str = (UA_String*)value.data;
            wchar_t wstr[256] = { 0 };
            if (str->data && str->length > 0) {
                MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)str->data, str->length, wstr, 255);
                swprintf_s(result + wcslen(result), 512 - wcslen(result), L"%s", wstr);
            }
            else {
                swprintf_s(result + wcslen(result), 512 - wcslen(result), L"(пустая строка)");
            }
        }
        else {
            swprintf_s(result + wcslen(result), 512 - wcslen(result), L"[Тип данных не поддерживается или неизвестен]");
        }

        for (int i = 0; i < 512; i++) {
            result_dec[i] = result[i];
        }

        SetWindowText(g_hEditInfo, L"Подключено успешно");
        //SetWindowText(g_hEditNumber, result_bool);
    }
    else {
        wchar_t errorMsg[256];
        swprintf_s(errorMsg, L"Ошибка чтения переменной: %S", UA_StatusCode_name(retval));
        SetWindowText(g_hEditInfo, errorMsg);
    }

    UA_Variant_clear(&value);  // Освобождение памяти, занятой вариантом
}
