#include <iostream>
#include <string>
#include <set>
#include <assert.h>
#include <Windows.h>

#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>

using string_t = std::basic_string<char_t>;

hostfxr_initialize_for_runtime_config_fn init_fptr;
hostfxr_get_runtime_delegate_fn get_delegate_fptr;
hostfxr_close_fn close_fptr;

void* load_library(const char_t* path)
{
    HMODULE h = ::LoadLibraryW(path);
    assert(h != nullptr);
    return (void*)h;
}
void* get_export(void* h, const char* name)
{
    void* f = ::GetProcAddress((HMODULE)h, name);
    assert(f != nullptr);
    return f;
}

bool load_hostfxr()
{
    // Pre-allocate a large buffer for the path to hostfxr
    char_t buffer[MAX_PATH];
    size_t buffer_size = sizeof(buffer) / sizeof(char_t);
    int rc = get_hostfxr_path(buffer, &buffer_size, nullptr);
    if (rc != 0)
        return false;

    // Load hostfxr and get desired exports
    void* lib = load_library(buffer);
    init_fptr = (hostfxr_initialize_for_runtime_config_fn)get_export(lib, "hostfxr_initialize_for_runtime_config");
    get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)get_export(lib, "hostfxr_get_runtime_delegate");
    close_fptr = (hostfxr_close_fn)get_export(lib, "hostfxr_close");

    return (init_fptr && get_delegate_fptr && close_fptr);
}

load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t* config_path)
{
    // Load .NET Core
    void* load_assembly_and_get_function_pointer = nullptr;
    hostfxr_handle cxt = nullptr;
    int rc = init_fptr(config_path, nullptr, &cxt);
    if (rc != 0 || cxt == nullptr)
    {
        std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
        close_fptr(cxt);
        return nullptr;
    }

    // Get the load assembly function pointer
    rc = get_delegate_fptr(
        cxt,
        hdt_load_assembly_and_get_function_pointer,
        &load_assembly_and_get_function_pointer);
    if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
        std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;

    close_fptr(cxt);
    return (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;
}

class Component
{
public:
    int id;
    const wchar_t* tag;

    Component(int id, const wchar_t* tag);
    ~Component();

};

Component::Component(int id, const wchar_t* tag) : id(id), tag(tag)
{
    std::cout << "\nComponent #" << id << " constructed in native code!" << std::endl;
}

Component::~Component()
{
    std::cout << "Component #" << id << " deconstructed in native code!" << std::endl;
}

std::set<Component*> componentPool;

Component* nativeComponentNew(int id, const wchar_t* tag)
{
    auto comp = new Component(id, tag);
    componentPool.insert(comp);
    return comp;
}

void nativeComponentDelete(Component* component)
{
    componentPool.erase(component);
    delete component;
}

void nativeComponentSetId(Component* component, int id)
{
    component->id = id;
}

int nativeComponentGetId(Component* component)
{
    return component->id;
}

void nativeComponentSetTag(Component* component, const wchar_t* tag)
{
    component->tag = tag;
}

const wchar_t* nativeComponentGetTag(Component* component)
{
    return component->tag;
}

struct InitPayload
{
    Component* (*nativeComponentNew)(int id, const wchar_t* tag);
    void (*nativeComponentDelete)(Component* component);
    void (*nativeComponentSetId)(Component* component, int id);
    int (*nativeComponentGetId)(Component* component);
    void (*nativeComponentSetTag)(Component* component, const wchar_t* tag);
    const wchar_t* (*nativeComponentGetTag)(Component* component);
};

void cleanUp()
{
    if (componentPool.size() != 0)
    {
        for (auto comp : componentPool)
        {
            std::cout << "WARNING: Component #" << comp->id << " constructed in native code (Should use using statement in managed code)" << std::endl;
            delete comp;
        }

        componentPool.clear();
    }
}

int main()
{
    if (!load_hostfxr())
    {
        assert(false && "Failure: load_hostfxr()");
        return EXIT_FAILURE;
    }

    const string_t root_path(L".\\");
    const string_t config_path = root_path + L"ManagedLib.runtimeconfig.json";
    load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;
    load_assembly_and_get_function_pointer = get_dotnet_load_assembly(config_path.c_str());
    assert(load_assembly_and_get_function_pointer != nullptr && "Failure: get_dotnet_load_assembly()");

    const string_t dotnetlib_path = root_path + L"ManagedLib.dll";
    const char_t* dotnet_type = L"ManagedLib.Entrance, ManagedLib";
    const char_t* dotnet_type_method = L"Init";

    typedef void (CORECLR_DELEGATE_CALLTYPE* entry_point_fn)(InitPayload payload);
    entry_point_fn Init = nullptr;
    int rc = load_assembly_and_get_function_pointer(
        dotnetlib_path.c_str(),
        dotnet_type,
        L"Init" /*method_name*/,
        UNMANAGEDCALLERSONLY_METHOD,
        nullptr,
        (void**)&Init);
    assert(rc == 0 && Init != nullptr && "Failure: load_assembly_and_get_function_pointer()");

    InitPayload payload{
        nativeComponentNew,
        nativeComponentDelete,
        nativeComponentSetId,
        nativeComponentGetId,
        nativeComponentSetTag,
        nativeComponentGetTag
    };

    Init(payload);
    std::cout << "----- Entrance Method Exited! -----" << std::endl;

    cleanUp();
    return EXIT_SUCCESS;
}