#pragma once

#include <string>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <fstream>
#include <iostream>
#include <comdef.h>
#include <exception>

#define DEFAULT_ENTRY_POINT "main"
#define DEFAULT_PS_SHADER_MODEL "ps_4_0"
#define DEFAULT_VS_SHADER_MODEL "vs_4_0"

class Shader
{
protected:
	std::string source;
	std::string entryPoint;
	std::string shaderModel;
	std::string fileName;
	ID3DBlob* shaderBlob;
public:
	void compile();
	void setEntryPoint(std::string entryPoint);
	void setShaderModel(std::string shaderModel);
	ID3DBlob* getBlobPointer();
	void loadShaderFromFile(std::string fileName);
private:
	void handleCompilationFailure(ID3DBlob* errorBlob, HRESULT shaderCompilatoinStatus);
};