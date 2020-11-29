#include "Shader.h"

void Shader::compile()
{
	UINT compilationFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	compilationFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* errorBlob = nullptr;
	HRESULT shaderCompilationStatus = D3DCompile2(source.c_str(), source.length(), nullptr, nullptr,
		nullptr, entryPoint.c_str(), shaderModel.c_str(), compilationFlags, 0, 0, nullptr, 0, &shaderBlob, &errorBlob);

	if (FAILED(shaderCompilationStatus))
	{
		handleCompilationFailure(errorBlob, shaderCompilationStatus);
	}
	else
	{
		std::cout << "Shader compilation succeded (" << fileName << ")." << std::endl;
	}
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

void Shader::loadShaderFromFile(std::string fileName)
{
	this->fileName = fileName;
	std::string line;
	std::ifstream shaderFile(fileName);
	if (shaderFile.is_open())
	{
		while (std::getline(shaderFile, line))
		{
			source += line + "\n";
		}
		shaderFile.close();
	}
	else
	{
		throw std::invalid_argument("File opening failure.");
	}
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
