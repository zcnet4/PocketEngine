//
//  ScriptWorld.cpp
//  ComponentSystem
//
//  Created by Jeppe Nielsen on 10/02/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include "ScriptWorld.hpp"
#include <dlfcn.h>
#include "ScriptParser.hpp"
#include <set>
#include <iostream>
#include <stdio.h>
#include "FileReader.hpp"

using namespace std;
using namespace Pocket;

ScriptWorld::ScriptWorld() : libHandle(0), baseSystemIndex(-1) {
    Types.Add<bool>();
    Types.Add<int>();
    Types.Add<float>();
    Types.Add<double>();
    Types.Add<std::string>();
}

string ScriptWorld::ExtractHeaderPath(const std::string &headerFile) {
    size_t lastPath = headerFile.rfind("/");
    return headerFile.substr(0, lastPath);
}

string ScriptWorld::ExtractHeaderName(const std::string &headerFile) {
    size_t lastPath = headerFile.rfind("/");
    return headerFile.substr(lastPath + 1, headerFile.size() - lastPath - 1);
}

void ScriptWorld::SetClangSdkPath(const std::string& clangSdkPath) {
    this->clangSdkPath = clangSdkPath;
}

void ScriptWorld::SetFiles(const std::string& dynamicLibPath, const std::string& scriptingIncludeDir, const std::vector<std::string> &sourceFiles, const std::vector<std::string> &headerFiles) {
    
    this->sourceFiles.clear();
    this->headerNames.clear();
    this->headerPaths.clear();
    
    this->dynamicLibPath = dynamicLibPath;
    this->scriptingIncludeDir = scriptingIncludeDir;
    this->sourceFiles = sourceFiles;
    
    for(auto& header : headerFiles) {
        headerNames.push_back(ExtractHeaderName(header));
    }
    
    //header paths
    {
        std::set<std::string> uniqueHeaderPaths;
        
        uniqueHeaderPaths.insert("-I" + scriptingIncludeDir);
        for(auto& header : headerFiles) {
            uniqueHeaderPaths.insert("-I" + ExtractHeaderPath(header));
        }
        
        for(auto& source : sourceFiles) {
            uniqueHeaderPaths.insert("-I" + ExtractHeaderPath(source));
        }
        
        for(auto& uniqueHeader : uniqueHeaderPaths) {
            headerPaths.push_back(uniqueHeader);
        }
    }
    
    /*
    std::cout << "headerPaths : "<<std::endl;
    for(auto& s : headerPaths) {
        std::cout << s << std::endl;
    }
    
    std::cout << "headerNames : "<<std::endl;
    for(auto& s : headerNames) {
        std::cout << s << std::endl;
    }
    */
}

bool ScriptWorld::Build(bool enableOutput, const std::string &pathToPocketEngineLib) {
    UnloadLib();
    std::string mainCppFile = scriptingIncludeDir + "/scriptWorld_autogenerated.cpp";
    ExtractScriptClasses();
    
    WriteMainCppFile(mainCppFile);
    
    WriteExecutableTypeInfos(scriptingIncludeDir + "/executable_autogenerated.hpp");
    
    string compilerPath = clangSdkPath + "bin/clang++";
    string compilerFlags = "-dynamiclib -std=c++11 "+pathToPocketEngineLib+" -stdlib=libc++ -g";
    if (enableOutput) {
        compilerFlags += " -v ";
    }
    string outputFile = "-o " + dynamicLibPath;
    
    string compilerArgs = compilerPath + " " + compilerFlags + " ";
    //compilerArgs += "-I"+clangSdkPath+"/include/c++/v1/ ";
    
    for(auto& header : headerPaths) {
        compilerArgs += header + " ";
    }
    compilerArgs += mainCppFile + " ";
    compilerArgs += outputFile;
    
    if (enableOutput) {
        std::cout << compilerArgs << std::endl;
    }
    
    //remove old library file
    FileReader::RunCommmand("rm " + dynamicLibPath);
    
    //remove old library dSYM folder
    std::string dsymFile = dynamicLibPath + ".dSYM";
    FileReader::RunCommmand("rm -r " + dsymFile);
    
    auto out = FileReader::RunCommmand(compilerArgs);
    
    if (enableOutput) {
        for(auto s : out) {
            std::cout << s << std::endl;
        }
    }
    return LoadLib();
}


bool ScriptWorld::BuildExecutable(const std::string &pathToPocketEngineLib) {

    //std::string mainCppFile = scriptingIncludeDir + "/scriptWorld_autogenerated.cpp";
    ExtractScriptClasses();
    
    //WriteMainCppFile(mainCppFile);
    
    std::string exeMainCpp = "main.cpp";
    
    WriteExecutableTypeInfos(scriptingIncludeDir + "/executable_autogenerated.hpp");
    
    WriteExecutableMain(exeMainCpp);
    
    //return false;
    
    string compilerPath = clangSdkPath + "bin/clang++";
    
    // -std=c++11 -stdlib=libc++ -DOSX libPocketEngine.a -o game -O3 -x objective-c++ -arch x86_64 -lobjc \
\
 -framework Foundation \
 -framework OpenGL \
 -framework IOKit \
 -framework Cocoa \
\

    
    string compilerFlags = "-std=c++14 -stdlib=libc++ -DOSX "+pathToPocketEngineLib+" -o game -x objective-c++ -arch x86_64 -lobjc ";
    compilerFlags += "-framework Foundation -framework OpenGL -framework IOKit -framework Cocoa ";
    compilerFlags += " " + exeMainCpp;
    compilerFlags += " -v ";
    std::string POCKET_PATH=" -I /Projects/PocketEngine/Pocket/";
    
    compilerFlags += POCKET_PATH + "ComponentSystem";
    compilerFlags += POCKET_PATH + "ComponentSystem/Meta";
    compilerFlags += POCKET_PATH + "OpenGL";
    compilerFlags += POCKET_PATH + "Math";
    compilerFlags += POCKET_PATH + "Data";
    compilerFlags += POCKET_PATH + "Core";
    compilerFlags += POCKET_PATH + "Platform/OSX";
    compilerFlags += POCKET_PATH + "Serialization";
    
    compilerFlags += POCKET_PATH + "Logic/Animation";
    compilerFlags += POCKET_PATH + "Logic/Audio";
    compilerFlags += POCKET_PATH + "Logic/Common";
    compilerFlags += POCKET_PATH + "Logic/Editor";
    compilerFlags += POCKET_PATH + "Logic/Effects";
    compilerFlags += POCKET_PATH + "Logic/Gui";
    compilerFlags += POCKET_PATH + "Logic/Interaction";
    
    compilerFlags += POCKET_PATH + "Logic/Movement";
    compilerFlags += POCKET_PATH + "Logic/Physics";
    compilerFlags += POCKET_PATH + "Logic/Rendering";
    compilerFlags += POCKET_PATH + "Logic/Selection";
    compilerFlags += POCKET_PATH + "Logic/Spatial";
    compilerFlags += POCKET_PATH + "Rendering/";
    
    
    
    
    for(auto& sourceFile : sourceFiles) {
        compilerFlags += " " + sourceFile;
    }
    compilerFlags += " ";
    
    
    string compilerArgs = compilerPath + " " + compilerFlags + " ";
    
    auto out = FileReader::RunCommmand(compilerArgs);

    for(auto s : out) {
        std::cout << s << std::endl;
    }
    
    //FileReader::RunCommmand(("./game");
    
    std::cout << "Build completed..."<<std::endl;
    
    return true;
}


bool ScriptWorld::LoadLib() {
    
    libHandle = dlopen(dynamicLibPath.c_str(), RTLD_LAZY);
    
    if (!libHandle) {
        cerr << "Cannot open '" << dynamicLibPath << "' library: " << dlerror() << '\n';
        return false;
    }

    createSystem = (CreateSystem)dlsym(libHandle, "CreateSystem");
    const char *dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'CreateSystem': " << dlsym_error << '\n';
        UnloadLib();
        return false;
    }
    
    countSystems = (CountSystems) dlsym(libHandle, "CountSystems");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'CountSystems': " << dlsym_error << '\n';
        UnloadLib();
        return false;
    }
    
    deleteSystem = (DeleteSystem) dlsym(libHandle, "DeleteSystem");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'DeleteSystem': " << dlsym_error << '\n';
        UnloadLib();
        return false;
    }
    
    createComponent = (CreateComponent) dlsym(libHandle, "CreateComponent");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'CreateComponent': " << dlsym_error << '\n';
        dlclose(libHandle);
        return false;
    }
    
    countComponents = (CountComponents) dlsym(libHandle, "CountComponents");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'CountComponents': " << dlsym_error << '\n';
        dlclose(libHandle);
        return false;
    }
    
    deleteComponent = (DeleteComponent) dlsym(libHandle, "DeleteComponent");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'DeleteComponent': " << dlsym_error << '\n';
        dlclose(libHandle);
        return false;
    }
    
    resetComponent = (ResetComponent) dlsym(libHandle, "ResetComponent");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'ResetComponent': " << dlsym_error << '\n';
        dlclose(libHandle);
        return false;
    }
    
    getTypeInfo = (GetTypeInfoFunction) dlsym(libHandle, "GetTypeInfo");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'GetTypeInfo': " << dlsym_error << '\n';
        dlclose(libHandle);
        return false;
    }
    
    deleteTypeInfo = (DeleteTypeInfo) dlsym(libHandle, "DeleteTypeInfo");
    dlsym_error = dlerror();
    if (dlsym_error) {
        cerr << "Cannot load symbol 'DeleteTypeInfo': " << dlsym_error << '\n';
        dlclose(libHandle);
        return false;
    }
    
    return true;
}

void ScriptWorld::UnloadLib() {
    if (!libHandle) return;
    dlclose(libHandle);
}

void ScriptWorld::ExtractScriptClasses() {
    ScriptClass allClasses;

    ScriptParser parser;

    parser.ParseCode(
        allClasses,
        sourceFiles,
        headerPaths
    );

    //allClasses.Print();
    scriptClasses = parser.ExtractSystemsAndComponents(allClasses);
}

void ScriptWorld::WriteMainCppFile(const std::string &path) {
    ofstream file;
    file.open(path);
    
    WriteMainGameObject(file);
    WriteMainIncludes(file);
    WriteTypes(file);
    WriteMainSystems(file);
    WriteMainComponents(file);
    WriteMainSerializedComponents(file);
    
    file.close();
}

void ScriptWorld::WriteMainIncludes(std::ofstream &file) {

    file<<"namespace Pocket { class IGameSystem; }"<<std::endl;
    
    auto& components = scriptClasses.children["Components"].children;
    auto& systems = scriptClasses.children["Systems"].children;
    
    std::set<std::string> uniqueIncludes;

    for(auto sf : sourceFiles) {
        uniqueIncludes.insert(ExtractHeaderName(sf));
    }
    
    for(auto& component : components) {
        uniqueIncludes.insert(component.second.sourceFile);
    }
    for(auto& system : systems) {
        uniqueIncludes.insert(system.second.sourceFile);
    }
    
    file<<"#include \"TypeInfo.hpp\""<<std::endl;
    
    for(auto& headerName : headerNames) {
        file<<"#include \""<<headerName<<"\""<<std::endl;
    }
    
    for(auto& include : uniqueIncludes) {
        file << "#include \"" << include << "\""<< std::endl;
    }
    file<<std::endl;
    
}

void ScriptWorld::WriteMainGameObject(std::ofstream &file) {
    
    const std::string namespaceName = "Pocket::";

    auto& scriptComponents = scriptClasses.children.find("Components")->second.children;
    
    file<<"namespace Pocket {"<<std::endl;
    for(auto& componentName : worldComponentNames) {
        file << "struct "<< componentName.name << ";"<<std::endl;
    }
    file<<"}"<<std::endl;
    
    //file<<"using namespace Pocket;"<<std::endl;
    
        
    for (auto component : scriptComponents) {
        file << "struct "<< component.second.name << ";"<<std::endl;
    }
    
    file << std::endl;
    
    file << "#include \"GameSystem.hpp\""<<std::endl;
    
    /*
    file << "namespace Pocket {"<<std::endl;
    file << "class GameObject {"<<std::endl;
    file << "private:"<<std::endl;
    file << "    virtual void* GetComponent(int componentID) = 0;"<<std::endl;
    file << "    virtual void AddComponent(int componentID) = 0;"<<std::endl;
    file << "    virtual void AddComponent(int componentID, GameObject* referenceObject) = 0;"<<std::endl;
    file << "    virtual void RemoveComponent(int componentID) = 0;"<<std::endl;
    file << "    virtual void CloneComponent(int componentID, GameObject* source) = 0;"<<std::endl;
    
    file << "public:" << std::endl;
    file << "    template<typename T> T* GetComponent() { return (T*)0; }"<<std::endl;
    file << "    template<typename T> T* AddComponent() { }"<<std::endl;
    file << "    template<typename T> void RemoveComponent() { }"<<std::endl;
    file << "    template<typename T> T* CloneComponent(GameObject* source) { }"<<std::endl;
    file << "};"<<std::endl;
    file << "}"<<std::endl;
    */
    
    for(auto& componentName : worldComponentNames) {
        std::string nameWithNamespace = namespaceName + componentName.name;
        file<<"template<> " << nameWithNamespace  << "* Pocket::GameObject::GetComponent<"<< nameWithNamespace << ">() { return ("<< nameWithNamespace <<"*) GetComponent("<<componentName.index<<"); }"<<std::endl;
        
        file<<"template<> " << nameWithNamespace  << "* Pocket::GameObject::AddComponent<"<< nameWithNamespace << ">() { AddComponent("<<componentName.index<<"); return ("<< nameWithNamespace <<"*) GetComponent("<<componentName.index<<"); }"<<std::endl;
     
        file<<"template<> void Pocket::GameObject::RemoveComponent<"<< nameWithNamespace << ">() { RemoveComponent("<<componentName.index<<"); }"<<std::endl;
        
        file<<"template<> " << nameWithNamespace  << "* Pocket::GameObject::CloneComponent<"<< nameWithNamespace << ">(GameObject* source) { CloneComponent("<<componentName.index<<", source); return ("<< nameWithNamespace <<"*) GetComponent("<<componentName.index<<"); }"<<std::endl;
    }
    
    int index = baseComponentIndex;
    for (auto it : scriptComponents) {
        auto& component = it.second;
        file<<"template<> " << component.name  << "* Pocket::GameObject::GetComponent<"<< component.name << ">() { return ("<< component.name <<"*) GetComponent("<<index<<"); }"<<std::endl;
        
        file<<"template<> " << component.name  << "* Pocket::GameObject::AddComponent<"<< component.name << ">() { AddComponent("<<index<<"); return ("<< component.name <<"*) GetComponent("<<index<<"); }"<<std::endl;
        
        file<<"template<> void Pocket::GameObject::RemoveComponent<"<< component.name << ">() { RemoveComponent("<<index<<"); }"<<std::endl;
        
        file<<"template<> " << component.name  << "* Pocket::GameObject::CloneComponent<"<< component.name << ">(GameObject* source) { CloneComponent("<<index<<", source); return ("<< component.name <<"*) GetComponent("<<index<<"); }"<<std::endl;
        
        index++;
    }
}

void ScriptWorld::WriteMainSystems(std::ofstream &file) {
    
    auto& systems = scriptClasses.children["Systems"].children;
    
    {
       file<<"extern \"C\" int CountSystems() {"<<std::endl;
       file<<"   return "<< systems.size() <<";"<<std::endl;
       file<<"}"<<std::endl;
    }
    
    {
        file<<"extern \"C\" IGameSystem* CreateSystem(int systemID) {"<<std::endl;
            file << "   switch (systemID) { " << std::endl;
                int index = baseSystemIndex;
                for(auto& system : systems) {
                    file<<"      case "<<index <<":"<<" return new "<<system.second.name<<"();"<<std::endl;
                    index++;
                }
        file<<"      default: return 0;"<<std::endl;
        file<<"   }"<<std::endl;
        file<<"}"<<std::endl;
    }
    
    {
       file<<"extern \"C\" void DeleteSystem(IGameSystem* scriptSystem) {"<<std::endl;
       file<<"   delete scriptSystem; "<<std::endl;
       file<<"}"<<std::endl;
    }
}

void ScriptWorld::WriteMainComponents(std::ofstream &file) {
    
    auto& components = scriptClasses.children["Components"].children;

    {
       file<<"extern \"C\" int CountComponents() {"<<std::endl;
       file<<"   return "<< components.size() <<";"<<std::endl;
       file<<"}"<<std::endl;
    }
    
    {
        file<<"extern \"C\" void* CreateComponent(int componentID) {"<<std::endl;
        file << "   switch (componentID) { " << std::endl;
        int index = baseComponentIndex;
        for(auto& component : components) {
            file<<"      case "<<index <<":"<<" return new "<<component.second.name<<"();"<<std::endl;
            index++;
        }
        file<<"      default: return 0;"<<std::endl;
        file<<"   }"<<std::endl;
        file<<"}"<<std::endl;
    }
    
    {
       file<<"extern \"C\" void DeleteComponent(int componentID, void* component) {"<<std::endl;
        file << "   switch (componentID) { " << std::endl;
        int index = baseComponentIndex;
        for(auto& component : components) {
            file<<"      case "<<index <<":"<<" { delete (("<<component.second.name<<"*)component); break; }"<<std::endl;
            index++;
        }
        file<<"   }"<<std::endl;
        file<<"}"<<std::endl;
    }
    
    {
       file<<"extern \"C\" void ResetComponent(int componentID, void* c, void* s) {"<<std::endl;
            file << "   switch (componentID) { " << std::endl;
                int index = baseComponentIndex;
                for(auto& component : components) {
                    file<<"      case "<<index <<":"<<" { "<<component.second.name<<"* co = ("<<component.second.name<<"*)c; "<<std::endl;
                    file<<"      "<<component.second.name<<"* so = (("<<component.second.name<<"*)s);"<<std::endl;
                    file<<"        co->operator=(*so);             break; }"<<std::endl;
                    index++;
                }
        file<<"   }"<<std::endl;
        file<<"}"<<std::endl;
    }

    file<<std::endl;
}

void ScriptWorld::WriteMainSerializedComponents(std::ofstream &file) {

    auto& components = scriptClasses.children["Components"].children;

    file<<"extern \"C\" Pocket::TypeInfo* GetTypeInfo(int componentID, void* componentPtr) {"<<std::endl;
        file << "   switch (componentID) { " << std::endl;
            int index = baseComponentIndex;
            for(auto& componentIt : components) {
                auto& component = componentIt.second;
                file<<"      case "<<index <<": {"<<std::endl;
                file<<"      "<<component.name<<"* component = ("<<component.name<<"*)componentPtr;"<<std::endl;
                file<<"	      Pocket::TypeInfo* info = new Pocket::TypeInfo();"<<std::endl;
                file<<"	      info->name = \""<<component.name<<"\";"<<std::endl;
                std::set<std::string> uniqueFields;
                for(auto& f : component.fields) {
                    if (!IsFieldValid(f)) continue;
                    uniqueFields.insert(f.name);
                }
                
                for(auto& field : uniqueFields) {
                    file<<"	      info->AddField(component->"<< field <<", \""<<field<<"\");"<<std::endl;
                }
                file<<"      return info;"<<std::endl;
                
                file<<"      break; }"<<std::endl;
                index++;
            }
    file<<"      default: return 0;"<<std::endl;
    file<<"   }"<<std::endl;
    file<<"}"<<std::endl;

    file<<"extern \"C\" void DeleteTypeInfo(Pocket::TypeInfo* typeInfo) {"<<std::endl;
    file << "delete typeInfo;"<<std::endl;
    file<<"}"<<std::endl;
}

void ScriptWorld::WriteTypes(std::ofstream &file) {

    file << "#include <string>"<<std::endl;
    file << "#include <vector>"<<std::endl;
    file << "#include \"Property.hpp\""<<std::endl;
    auto typeNames = Types.GetTypeNames();
    
    for(auto n : typeNames) {
        file<<"template<> struct Pocket::FieldInfoIndexer<"<< n.second << "> { static int Index() { return "<< n.first << "; } };"<<std::endl;
    }
    file<<""<<std::endl;
}

void ScriptWorld::WriteExecutableMain(const std::string &path) {
    
    auto& systems = scriptClasses.children["Systems"].children;
    
    ofstream file;
    file.open(path);
    
    file<<"#include \"OpenGL.hpp\""<<std::endl;
    file<<"#include \"Engine.hpp\""<<std::endl;
    file<<"#include \"GameWorld.hpp\""<<std::endl;
    file<<"#include \"RenderSystem.hpp\""<<std::endl;
    file<<"#include \"TouchSystem.hpp\""<<std::endl;
    file<<"#include \"TouchSystem.hpp\""<<std::endl;
    file<<"#include <iostream>"<<std::endl;
    file<<"#include <fstream>"<<std::endl;
    
    for(auto& source : sourceFiles) {
        file<<"#include \""<<source<<"\""<<std::endl;
    }
    file<<"using namespace Pocket;"<<std::endl;

    file<<"struct Game : public GameState<Game> {"<<std::endl;
    file<<"    GameWorld world;"<<std::endl;
    file<<"    void Initialize() { "<<std::endl;
    
    file<<"         #include \""<< (scriptingIncludeDir + "/executable_autogenerated.hpp") <<"\""<<std::endl;
    
    file<<"         world.CreateSystem<RenderSystem>(); "<<std::endl;
    file<<"         world.CreateSystem<TouchSystem>()->Input = &Input; "<<std::endl;
    
    for(auto& system : systems) {
        file<<"         world.CreateSystem<"<<system.first<<">();"<<std::endl;
    }
    file<<"     "<<std::endl;
    file<<" std::ifstream file;"<<std::endl;
    file<<" file.open(\"world.json\");  "<<std::endl;
    file<<" world.CreateObject(file);"<<std::endl;
    file<<" file.close();"<<std::endl;
    
    file<<"     "<<std::endl;
    file<<"    } "<<std::endl;
    
    file<<"    void Update(float dt) { world.Update(dt); }"<<std::endl;
    file<<"    void Render() {"<<std::endl;
    file<<"        glClearColor(1,0,0,0);"<<std::endl;
    file<<"        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);"<<std::endl;
    file<<"        world.Render(); "<<std::endl;
    file<<"    }"<<std::endl;
    file<<"};"<<std::endl;

    file<<"int main(int argc, const char * argv[]) {"<<std::endl;
    file<<"    Engine e;"<<std::endl;
    file<<"    e.Start<Game>();"<<std::endl;
    file<<"    return 0;"<<std::endl;
    file<<"}"<<std::endl;
    
    file.close();

}


void ScriptWorld::WriteExecutableTypeInfos(const std::string &path) {
    
    auto& components = scriptClasses.children["Components"].children;
    
    ofstream file;
    file.open(path);
    
    file<<"GameWorld::OnGetTypeInfo = [] (int componentID, GameObject::ComponentInfo& info) {"<<std::endl;
        file<<" static std::map<int, std::function<TypeInfo(GameObject*)>> componentToFunction; "<<std::endl;
        file<<" if (!componentToFunction.empty()) {" <<std::endl;
    
            for(auto& componentIt : components) {
                auto& component = componentIt.second;
                    file << "componentToFunction[GameIDHelper::GetComponentID<"<<component.name<<">()] = [](GameObject* object) -> TypeInfo {"<<std::endl;
                        file<<component.name<<"* component = object->GetComponent<"<<component.name<<">();"<<std::endl;
                        file<< "TypeInfo typeInfo;"<<std::endl;
                        file<<"	      typeInfo.name = \""<<component.name<<"\";"<<std::endl;
                        std::set<std::string> uniqueFields;
                        for(auto& f : component.fields) {
                            if (!IsFieldValid(f)) continue;
                            uniqueFields.insert(f.name);
                        }
                        
                        for(auto& field : uniqueFields) {
                            file<<"	      typeInfo.AddField(component->"<< field <<", \""<<field<<"\");"<<std::endl;
                        }
                
                        file<< "return typeInfo;"<<std::endl;
                    file<<"};"<<std::endl;
                
            }
        file<<"   }"<<std::endl;
    
        file<<"info.getTypeInfo = componentToFunction[componentID];"<<std::endl;
    
    file<<"};"<<std::endl;
    file.close();
}

bool ScriptWorld::IsFieldValid(const ScriptClass::Field &field) {
    return field.type!="<";
}

bool ScriptWorld::FindComponentIndex(std::string componentName, bool &staticComponent, int& index) {
    auto& scriptComponents = scriptClasses.children["Components"].children;
    {
        index = 0;
        for(auto& scriptComponent : scriptComponents) {
            if (scriptComponent.second.name == componentName) {
                staticComponent = false;
                return true;
            }
            index++;
        }
    }
    
    index = -1;
    
    for(auto& worldComponentName : worldComponentNames) {
        if (componentName == worldComponentName.name) {
            index = worldComponentName.index;
            break;
        }
    }
    
    if (index>=0) {
        staticComponent = true;
        return true;
    }
    
    return false;
}

void ScriptWorld::SetWorldType(GameWorld& world) {
    worldComponentNames.clear();
    for(int i=0; i<world.components.size(); ++i) {
        std::string& name = world.components[i].name;
        size_t namespaceColons = name.find("::");
        std::string nameWithoutNamespace;
        if (namespaceColons!=std::string::npos) {
            nameWithoutNamespace = name.substr(namespaceColons+2, name.size() - namespaceColons-2);
        } else {
            nameWithoutNamespace = name;
        }
        if (nameWithoutNamespace!="") {
            worldComponentNames.push_back({ nameWithoutNamespace, i });
        }
    }
    
    baseComponentIndex = world.components.size();
    baseSystemIndex = world.systems.size();
}

bool ScriptWorld::AddGameWorld(GameWorld& world) {
    if (!libHandle) return false;
    
    scriptComponents.clear();
    
    int numberOfComponents = countComponents();
    componentCount = numberOfComponents;

    assert(baseComponentIndex == (int)world.components.size());
    assert(baseSystemIndex == (int)world.systems.size());
    
    
    for(int i=0; i<numberOfComponents; ++i) {
        int componentIndex = baseComponentIndex + i;
        world.AddComponentType(componentIndex, [this, componentIndex](GameWorld::ComponentInfo& componentInfo) {
            Container<ScriptComponent>* container = new Container<ScriptComponent>();
            container->defaultObject.world = this;
            container->defaultObject.componentID = componentIndex;
            container->defaultObject.data = createComponent(componentIndex);
            componentInfo.getTypeInfo = [this, componentIndex](GameObject* object) -> TypeInfo {
                return GetTypeInfo(*object, componentIndex);
            };
            componentInfo.container = container;
            return container;
        });
        
        auto& components = scriptClasses.children["Components"].children;
        int componentNameCounter = 0;
        for(auto c : components) {
            if (componentNameCounter == i) {
                world.components[componentIndex].name = c.second.name;
                scriptComponents[c.second.name] = componentIndex;
            }
            componentNameCounter++;
        }
    }

    
    auto& scriptSystems = scriptClasses.children["Systems"].children;
    
    int index = 0;
    for (auto& scriptSystem : scriptSystems) {
        int systemIndex = baseSystemIndex + index;
        world.AddSystemType(systemIndex, [this, &scriptSystem, systemIndex] (GameWorld::SystemInfo& systemInfo, std::vector<ComponentId>& components) {
            
            for (auto& component : scriptSystem.second.templateArguments) {
                int componentIndex;
                bool staticComponent;
                if (FindComponentIndex(component, staticComponent, componentIndex)) {
                    components.push_back(staticComponent ? componentIndex : (baseComponentIndex + componentIndex));
                }
            }
            systemInfo.createFunction = [this, systemIndex] (GameObject* root) {
                return createSystem(systemIndex);
            };
            systemInfo.deleteFunction = [this, systemIndex] (IGameSystem* system) {
                deleteSystem(system);
            };
        });
        index++;
    }
    return true;
}

void ScriptWorld::RemoveGameWorld(GameWorld& world) {
    if (baseSystemIndex == -1) return;
    int endSystemIndex = (int)world.systems.size();
    for(int i=baseSystemIndex; i<endSystemIndex; ++i){
        world.RemoveSystemType(i);
    }
    world.systems.resize(endSystemIndex);
    int endComponentIndex = (int)world.components.size();
    for(int i=baseComponentIndex; i<endComponentIndex; ++i) {
        delete world.components[i].container;
    }
    world.components.resize(baseComponentIndex);
    world.objects.Iterate([this](GameObject* o) {
        o->activeComponents.Resize(baseComponentIndex);
        o->enabledComponents.Resize(baseComponentIndex);
    });
    world.components.resize(baseComponentIndex);
    for(int i=0; i<baseComponentIndex; ++i) {
        world.components[i].systemsUsingComponent.resize(baseComponentIndex);
    }
}

TypeInfo ScriptWorld::GetTypeInfo(GameObject& object, ComponentId id) {
    void* component = object.GetComponent(id);
    if (!component) {
        return TypeInfo();
    }
    TypeInfo* info = getTypeInfo(id, component);
    TypeInfo t;
    t.UpdateFromPointer(info, Types);
    deleteTypeInfo(info);
    return t;
}

int ScriptWorld::ComponentCount() { return componentCount; }

ScriptWorld::ScriptComponents ScriptWorld::Components() {
    return scriptComponents;
}

template<> void* Container<ScriptComponent>::Get(int index) {
    return entries[index].data;
}




