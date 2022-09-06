/**
 * @file config.h
 * @author your name (you@domain.com)
 * @brief 配置模块
 * @version 0.1
 * @date 2022-08-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __SERVER_CONFIG_H__
#define __SERVER_CONFIG_H__

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <boost/lexical_cast.hpp>
#include <yaml-cpp/yaml.h>
#include "../log/log.h"
#include "../public/util.h"

namespace server
{
	/**
	 * @brief 配置变量的基类, 抽象类，仅提供接口
	 * 
	 */
	class ConfigVarBase
	{
	public:
		typedef std::shared_ptr<ConfigVarBase> ptr;

		/**
		 * @brief Construct a new Config Var Base object
		 * 
		 * @param name 配置参数名称
		 * @param description 配置参数描述 
		 */
		ConfigVarBase(const std::string& name, const std::string& description = "")
			: m_name(name), m_description(description) 
		{ std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower); }

		/**
		 * @brief Destroy the Config Var Base object
		 * @note 虚析构函数，防止内存泄露
		 */
		virtual ~ConfigVarBase() {}

		/**
		 * @brief 返回配置参数名称
		 * 
		 * @return const std::string& 
		 */
		const std::string& getName() const { return m_name; }

		/**
		 * @brief 返回配置参数的描述
		 * 
		 * @return const std::string& 
		 */
		const std::string& getDescription() const { return m_description; }
		
		/**
		 * @brief 转成字符串, 纯虚函数
		 * 
		 * @return std::string 
		 */
		virtual std::string toString() = 0;
		virtual bool fromString(const std::string& val) = 0;
		virtual std::string getTypeName() const = 0;
	protected:
		std::string m_name;						// 配置参数的名称
		std::string m_description;				// 配置参数的描述
	};

	/******************************** 伪函数  ******************/

	/**
	 * @brief 类型转换模板类(F 源类型, T 目标类型)
	*/
	template<class F, class T>
	class LexicalCast
	{
	public:
		/**
		 * @brief 类型转换
		 * @param[in] v 源类型值
		 * @return 返回v转换后的目标类型
		 * @exception 当类型不可转换时抛出异常
		 */
		T operator() (const F& v) 
		{
			return boost::lexical_cast<T>(v);
		}

	};

	/**
	 * @brief 类型转换模板类偏特化(YAML String 转换成 std::vector<T>)
	 */
	template<class T>
	class LexicalCast<std::string, std::vector<T> >
	{
	public:
		std::vector<T> operator() (const std::string& v)
		{
			YAML::Node node = YAML::Load(v);
			// 确保引用模板中的类型，模板在实例化之前并不知道std::vector<T>是什么，使用typename可以让定义确定
			typename std::vector<T> vec;
			std::stringstream ss;
			for(size_t i = 0; i < node.size(); ++i)
			{
				ss.str("");
				ss << node[i];
				vec.push_back(LexicalCast<std::string, T>() (ss.str()));
			}
			return vec;
		}
	};

	/**
	 * @brief 类型转换模板类偏特化(std::vector<T> 转换成 YAML String)
	 */
	template<class T>
	class LexicalCast<std::vector<T>, std::string >
	{
	public:
		std::string operator() (const std::vector<T>& v)
		{
			YAML::Node node;
			for(auto& i : v)
			{
				node.push_back( YAML::Load(LexicalCast<T, std::string>() (i)) );
			}
			std::stringstream ss;
			ss << node;
			return ss.str();
		}
	};

 	/**
	 * @brief 类型转换模板类偏特化(YAML String 转换成 std::list<T>)
	 */
	template<class T>
	class LexicalCast<std::string, std::list<T> >
	{
	public:
		std::list<T> operator() (const std::string& v)
		{
			YAML::Node node = YAML::Load(v);
			typename std::list<T> vec;
			std::stringstream ss;
			for(size_t i = 0; i < node.size(); ++i)
			{
				ss.str("");
				ss << node[i];
				vec.push_back(LexicalCast<std::string, T>() (ss.str()));
			}
			return vec;
		}
	};

	/**
	 * @brief 类型转换模板类偏特化(std::list<T> 转换成 YAML String)
	 */
	template<class T>
	class LexicalCast<std::list<T>, std::string >
	{
	public:
		std::string operator() (const std::list<T>& v)
		{
			YAML::Node node;
			for(auto& i : v)
			{
				node.push_back( YAML::Load(LexicalCast<T, std::string>() (i)) );
			}
			std::stringstream ss;
			ss << node;
			return ss.str();
		}
	};

 	/**
	 * @brief 类型转换模板类偏特化(YAML String 转换成 std::set<T>)
	 */
	template<class T>
	class LexicalCast<std::string, std::set<T> >
	{
	public:
		std::set<T> operator() (const std::string& v)
		{
			YAML::Node node = YAML::Load(v);
			typename std::set<T> vec;
			std::stringstream ss;
			for(size_t i = 0; i < node.size(); ++i)
			{
				ss.str("");
				ss << node[i];
				vec.insert(LexicalCast<std::string, T>() (ss.str()));
			}
			return vec;
		}
	};

	/**
	 * @brief 类型转换模板类偏特化(std::set<T> 转换成 YAML String)
	 */
	template<class T>
	class LexicalCast<std::set<T>, std::string >
	{
	public:
		std::string operator() (const std::set<T>& v)
		{
			YAML::Node node;
			for(auto& i : v)
			{
				node.push_back( YAML::Load(LexicalCast<T, std::string>() (i)) );
			}
			std::stringstream ss;
			ss << node;
			return ss.str();
		}
	};
	
 	/**
	 * @brief 类型转换模板类偏特化(YAML String 转换成 std::unordered_set<T>)
	 */
	template<class T>
	class LexicalCast<std::string, std::unordered_set<T> >
	{
	public:
		std::unordered_set<T> operator() (const std::string& v)
		{
			YAML::Node node = YAML::Load(v);
			// 确保引用模板中的类型，模板在实例化之前并不知道std::vector<T>是什么，使用typename可以让定义确定
			typename std::unordered_set<T> vec;
			std::stringstream ss;
			for(size_t i = 0; i < node.size(); ++i)
			{
				ss.str("");
				ss << node[i];
				vec.insert(LexicalCast<std::string, T>() (ss.str()));
			}
			return vec;
		}
	};

	/**
	 * @brief 类型转换模板类偏特化(std::unordered_set<T> 转换成 YAML String)
	 */
	template<class T>
	class LexicalCast<std::unordered_set<T>, std::string >
	{
	public:
		std::string operator() (const std::unordered_set<T>& v)
		{
			YAML::Node node;
			for(auto& i : v)
			{
				node.push_back( YAML::Load(LexicalCast<T, std::string>() (i)) );
			}
			std::stringstream ss;
			ss << node;
			return ss.str();
		}
	};

 	/**
	 * @brief 类型转换模板类偏特化(YAML String 转换成 std::map<std::string, T>)
	 */
	template<class T>
	class LexicalCast<std::string, std::map<std::string, T> >
	{
	public:
		std::map<std::string, T> operator() (const std::string& v)
		{
			YAML::Node node = YAML::Load(v);
			typename std::map<std::string, T> vec;
			std::stringstream ss;
			for(auto it = node.begin(); it != node.end(); ++it)
			{
				ss.str("");
				ss << it->second;
				vec.insert(std::make_pair(it->first.Scalar(),
                        LexicalCast<std::string, T>()(ss.str())));
			}
			return vec;
		}
	};

	/**
	 * @brief 类型转换模板类偏特化(std::map<std::string, T> 转换成 YAML String)
	 */
	template<class T>
	class LexicalCast<std::map<std::string, T>, std::string >
	{
	public:
		std::string operator() (const std::map<std::string, T>& v)
		{
			YAML::Node node(YAML::NodeType::Map);
			for(auto& i : v)
			{
				node[i.first] = YAML::Load(LexicalCast<T, std::string>() (i.second));
			}
			std::stringstream ss;
			ss << node;
			return ss.str();
		}
	};
	
 	/**
	 * @brief 类型转换模板类偏特化(YAML String 转换成 std::unordered_map<std::string, T>)
	 */
	template<class T>
	class LexicalCast<std::string, std::unordered_map<std::string, T> >
	{
	public:
		std::unordered_map<std::string, T> operator() (const std::string& v)
		{
			YAML::Node node = YAML::Load(v);
			typename std::unordered_map<std::string, T> vec;
			std::stringstream ss;
			for(auto it = node.begin(); it != node.end(); ++it)
			{
				ss.str("");
				ss << it->second;
				vec.insert(std::make_pair(it->first.Scalar(),
                        LexicalCast<std::string, T>()(ss.str())));
			}
			return vec;
		}
	};

	/**
	 * @brief 类型转换模板类偏特化(std::unordered_map<std::string, T> 转换成 YAML String)
	 */
	template<class T>
	class LexicalCast<std::unordered_map<std::string, T>, std::string >
	{
	public:
		std::string operator() (const std::unordered_map<std::string, T>& v)
		{
			YAML::Node node(YAML::NodeType::Map);
			for(auto& i : v)
			{
				node[i.first] = YAML::Load(LexicalCast<T, std::string>() (i.second));
			}
			std::stringstream ss;
			ss << node;
			return ss.str();
		}
	};
	// FromStr T operator() (std::string&)
	// ToStr std::string operator() (const T&)
	/**
	 * @brief 配置参数模板子类，保存对应类型的参数值
	 * 
	 * @tparam T 参数的具体类型
	 */
	template<class T, class FromStr = LexicalCast<std::string, T>
				    , class ToStr = LexicalCast<T, std::string> >
	class ConfigVar : public ConfigVarBase
	{
	public:
		typedef std::shared_ptr<ConfigVar> ptr;
		typedef std::function<void (const T& old_value, const T& new_value)> on_change_cb;

		/**
		 * @brief 通过参数名,参数值,描述构造ConfigVar
		 * @param[in] name 参数名称有效字符为[0-9a-z_.]
		 * @param[in] default_value 参数的默认值
		 * @param[in] description 参数的描述
		 */
		ConfigVar(const std::string& name,
				  const T& default_value, 
				  const std::string& description = "")
				: ConfigVarBase(name, description)
				, m_val(default_value) 
		{ std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower); }
		
		std::string toString() override
		{
			try 
			{
				// return boost::lexical_cast<std::string>(m_val);
				return ToStr() (m_val);
			} 
			catch (std::exception& e) 
			{
				SERVER_LOG_ERROR(SERVER_LOG_ROOT()) << "ConfigVar::toString exception " 
							<<  e.what() << "convert: " << TypeToName<T>() << " to string"
							<< " name=" << m_name;
			}
			return "";
		}

		bool fromString(const std::string& val) override
		{
			try
			{
				setValue(FromStr()(val));
			}
			catch(const std::exception& e)
			{
				SERVER_LOG_ERROR(SERVER_LOG_ROOT()) << "ConfigVar::fromString exception " 
						<<  e.what() << " convert: string to " << TypeToName<T>()
						<< " name=" << m_name << " - " << val;			
			}

			return false;
		}

		const T getValue() const { return m_val; }
		void setValue(const T& v) 
		{
			if(v == m_val)
			{
				return;
			}
			for(auto& i : m_cbs)
			{
				i.second(m_val, v);
			}
			m_val = v;
		}

		std::string getTypeName() const override { return typeid(T).name(); }

		/**
		 * @brief 增加监听器
		 * @param key 关键词，唯一标识
		 * @param cb 回调函数
		*/
		void addListener(uint64_t key, on_change_cb cb)
		{
			m_cbs[key] = cb;
		}

		void delListener(uint64_t key, on_change_cb cb)
		{
			m_cbs.erase(key);
		}

		on_change_cb getListener(uint64_t key)
		{
			auto it = m_cbs.find(key);
			return it == m_cbs.end() ? nullptr : it->second;
		}

		void clearListener()
		{
			m_cbs.clear();
		}
	private:
		T m_val;

		// 变更回调函数, uint64_t key
		std::map<uint64_t, on_change_cb> m_cbs;
	};

	/**
	 * @brief ConfigVar的管理类, 提供便捷的方法创建和访问ConfigVar
	 */
	class Config
	{
	public:
		// 为何要定义基类指针？
		typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;

		/**
		 * @brief 获取/创建对应参数名的配置参数
		 * @param[in] name 配置参数名称
		 * @param[in] default_value 参数默认值
		 * @param[in] description 参数描述
		 * @details 获取参数名为name的配置参数,如果存在直接返回
		 *          如果不存在,创建参数配置并用default_value赋值
		 * @return 返回对应的配置参数,如果参数名存在但是类型不匹配则返回nullptr
		 * @exception 如果参数名包含非法字符[^0-9a-z_.] 抛出异常 std::invalid_argument
		 */
		template<class T>
		static typename ConfigVar<T>::ptr Lookup(const std::string& name,
				const T& default_value, const std::string& description = "") 
		{	
			// std::cout << "m_datas size: " << GetDatas().size() << std::endl;
			auto it = GetDatas().find(name);
			if(it != GetDatas().end())
			{
				// 模板类，不同数据类型指针不一样
				auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
				if(tmp)
				{
					SERVER_LOG_INFO(SERVER_LOG_ROOT()) << "Lookup name = " << name << " exist";
					return tmp;
				}
				else
				{
					SERVER_LOG_ERROR(SERVER_LOG_ROOT()) << "Lookup name = " << name << " exist but type is not " 
						<< typeid(T).name() << " real_type= " << it->second->getTypeName() << " "
						<< it->second->toString();
					return nullptr;
				}
			}

			// 查看是否存在非法字符
			if ( name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789")
				!= std::string::npos)
			{
				SERVER_LOG_ERROR(SERVER_LOG_ROOT()) << "Lookup name invalid " << name;
				throw std::invalid_argument(name);
			}

			// 创建配置参数派生类对象指针
			typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
			GetDatas()[name] = v;
			return v;
		}

		/**
		 * @brief 查找配置参数
		 * 
		 * @tparam T 
		 * @param name 配置参数的名称
		 * @return ConfigVar<T>::ptr 返回名为name的配置参数
		 */
		template<class T>
		static typename ConfigVar<T>::ptr Lookup(const std::string& name)
		{
			auto it = GetDatas().find(name);
			// 如果没找到，返回空指针
			if(it == GetDatas().end())
			{
				return nullptr;
			}
			// 找到了将基类指针转换成派生类指针
			return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
		}

		/**
		 * @brief 使用YAML::Node初始化配置模块
		 * 
		 * @param root 
		 */
		static void LoadFromYaml(const YAML::Node& root);

		/**
		 * @brief 查找配置参数，返回配置参数的基类
		 * 
		 * @param name 配置参数名称
		 * @return ConfigVarBase::ptr 
		 */
		static ConfigVarBase::ptr LookupBase(const std::string& name);
	private:
		// 静态成员变量，保存配置参数的信息，被该类的所有对象共享
		// 静态成员函数和静态成员变量初始化顺序无法确定，直接定义静态成员变量会导致错误
		// static ConfigVarMap m_datas;

		static ConfigVarMap& GetDatas()
		{
			static ConfigVarMap s_datas;
			return s_datas;
		}
	};
}

#endif