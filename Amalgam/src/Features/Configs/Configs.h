#pragma once
#include "../../SDK/SDK.h"

#include <filesystem>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

class CConfigs
{
	void SaveJson(boost::property_tree::ptree& mapTree, const char* name, bool val);
	void SaveJson(boost::property_tree::ptree& mapTree, const char* name, int val);
	void SaveJson(boost::property_tree::ptree& mapTree, const char* name, float val);
	void SaveJson(boost::property_tree::ptree& mapTree, const char* name, const IntRange_t& val);
	void SaveJson(boost::property_tree::ptree& mapTree, const char* name, const FloatRange_t& val);
	void SaveJson(boost::property_tree::ptree& mapTree, const char* name, const std::string& val);
	void SaveJson(boost::property_tree::ptree& mapTree, const char* name, const std::vector<std::string>& val);
	void SaveJson(boost::property_tree::ptree& mapTree, const char* name, const Color_t& val);
	void SaveJson(boost::property_tree::ptree& mapTree, const char* name, const Gradient_t& val);
	void SaveJson(boost::property_tree::ptree& mapTree, const char* name, const DragBox_t& val);
	void SaveJson(boost::property_tree::ptree& mapTree, const char* name, const WindowBox_t& val);

	void LoadJson(boost::property_tree::ptree& mapTree, const char* name, bool& val);
	void LoadJson(boost::property_tree::ptree& mapTree, const char* name, int& val);
	void LoadJson(boost::property_tree::ptree& mapTree, const char* name, float& val);
	void LoadJson(boost::property_tree::ptree& mapTree, const char* name, IntRange_t& val);
	void LoadJson(boost::property_tree::ptree& mapTree, const char* name, FloatRange_t& val);
	void LoadJson(boost::property_tree::ptree& mapTree, const char* name, std::string& val);
	void LoadJson(boost::property_tree::ptree& mapTree, const char* name, std::vector<std::string>& val);
	void LoadJson(boost::property_tree::ptree& mapTree, const char* name, Color_t& val);
	void LoadJson(boost::property_tree::ptree& mapTree, const char* name, Gradient_t& val);
	void LoadJson(boost::property_tree::ptree& mapTree, const char* name, DragBox_t& val);
	void LoadJson(boost::property_tree::ptree& mapTree, const char* name, WindowBox_t& val);

public:
	CConfigs();

	bool SaveConfig(const std::string& configName, bool bNotify = true);
	bool LoadConfig(const std::string& configName, bool bNotify = true);
	bool SaveVisual(const std::string& configName, bool bNotify = true);
	bool LoadVisual(const std::string& configName, bool bNotify = true);
	void RemoveConfig(const std::string& configName);
	void RemoveVisual(const std::string& configName);

	boost::property_tree::ptree ColorToTree(const Color_t& color);
	void TreeToColor(const boost::property_tree::ptree& tree, Color_t& out);

	std::string sCurrentConfig = "default";
	std::string sCurrentVisuals = "default";
	std::string sConfigPath;
	std::string sVisualsPath;
	const std::string sConfigExtension = ".json";
};

ADD_FEATURE(CConfigs, Configs);