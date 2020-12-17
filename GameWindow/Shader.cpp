#include "Shader.h"

Shader::Shader(std::string name, std::string entryPoint, std::string shaderModel) : fileName(name), entryPoint(entryPoint), shaderModel(shaderModel)
{
}

Shader::Shader() : fileName("Not-Set"), entryPoint("Not-Set"), shaderModel("Not-Set")
{
}

void Shader::setEntryPoint(std::string entryPoint)
{
	this->entryPoint = entryPoint;
}

void Shader::setShaderModel(std::string shaderModel)
{
	this->shaderModel = shaderModel;
}

ID3DBlob* Shader::getBlobPointer()
{
	return shaderBlob;
}

LPVOID Shader::GetBufferPointer()
{
	return shaderBlob->GetBufferPointer();
}

SIZE_T Shader::GetBufferSize()
{
	return shaderBlob->GetBufferSize();
}

void Shader::compileShaderFromFile(std::wstring fileName)
{
	std::cout << "Shader::compileShaderFromFile()" << std::endl;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	ID3DBlob* errorBlob = nullptr;

	HRESULT shaderCompilationStatus = D3DCompileFromFile(fileName.c_str(),
		nullptr, nullptr, entryPoint.c_str(), shaderModel.c_str(), compileFlags, 0, &shaderBlob, &errorBlob);
	handleCompilationFailure(errorBlob, shaderCompilationStatus);
}

void Shader::handleCompilationFailure(ID3DBlob* errorBlob, HRESULT shaderCompilatoinStatus)
{
	if (errorBlob)
	{
		std::cout << "ERROR: Shader compilation failed!" << std::endl;
		LPCSTR errorMessage = _com_error(shaderCompilatoinStatus).ErrorMessage();
		std::cout << errorMessage << std::endl;

		std::string title = "Shader " + fileName;
		MessageBox(nullptr, (char*)errorBlob->GetBufferPointer(), title.c_str(), MB_OK);
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		errorBlob->Release();
	}
}
