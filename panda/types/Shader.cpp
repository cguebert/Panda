#include <panda/types/Shader.h>
#include <panda/helper/ShaderCache.h>

#include <panda/DataFactory.h>
#include <panda/Data.inl>

#include <QOpenGLShaderProgram>

namespace panda
{

namespace types
{

Shader::Shader()
{ }

void Shader::clear()
{
	m_sourcesMap.clear();
}

void Shader::addSource(QOpenGLShader::ShaderType type, QString sourceCode)
{
	ShaderSource shaderSource;
	shaderSource.type = type;
	shaderSource.sourceCode = sourceCode.toLatin1();
	shaderSource.hash = qHash(shaderSource.sourceCode);
	m_sourcesMap[type] = shaderSource;
}

void Shader::removeSource(QOpenGLShader::ShaderType type)
{
	m_sourcesMap.remove(type);
}

void Shader::apply(QOpenGLShaderProgram& program) const
{
	QList<QOpenGLShader*> currentShaders = program.shaders();
	QList<QOpenGLShader*> newShaders;
	helper::ShaderCache* shaderCache = helper::ShaderCache::getInstance();

	// Get shader pointers from the cache
	for(const ShaderSource& source : m_sourcesMap.values())
		newShaders.push_back(shaderCache->getShader(source.type, source.sourceCode, source.hash));

	bool needLink = false;
	// Removing from the program the shader we do not want
	for(QOpenGLShader* shader : currentShaders)
	{
		if(!newShaders.contains(shader))
		{
			program.removeShader(shader);
			needLink = true;
		}
	}

	// Adding the one the program does not have yet
	for(QOpenGLShader* shader : newShaders)
	{
		if(!currentShaders.contains(shader))
		{
			program.addShader(shader);
			needLink = true;
		}
	}

	if(needLink)
		program.link();
}

const QList<Shader::ShaderSource> Shader::getSources() const
{
	return m_sourcesMap.values();
}

//***************************************************************//

template<> QString DataTrait<Shader>::valueTypeName() { return "shader"; }

template<>
void DataTrait<Shader>::writeValue(QDomDocument& doc, QDomElement& elem, const Shader& v)
{
	auto sources = v.getSources();

	for(const auto& source : sources)
	{
		QDomElement sourceNode = doc.createElement("Source");
		elem.appendChild(sourceNode);
		sourceNode.setAttribute("type", (int)source.type);

		QDomText node = doc.createTextNode(source.sourceCode);
		sourceNode.appendChild(node);
	}
}

template<>
void DataTrait<Shader>::readValue(QDomElement& elem, Shader& v)
{
	v.clear();

	QDomElement sourceNode = elem.firstChildElement("Source");
	while(!sourceNode.isNull())
	{
		int type = sourceNode.attribute("type").toInt();
		v.addSource(QOpenGLShader::ShaderType(type), sourceNode.text());
		sourceNode = sourceNode.nextSiblingElement("Source");
	}
}

//***************************************************************//

template class Data<Shader>;
template class Data< QVector<Shader> >;

int shaderDataClass = RegisterData< Shader >();
int shaderVectorDataClass = RegisterData< QVector<Shader> >();

} // namespace types

} // namespace panda
