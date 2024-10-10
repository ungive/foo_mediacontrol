#pragma once

// titleformat_object extracted from titleformat.h as it's more commonly used than other titleformat.h stuff

class file_info; class titleformat_hook; class titleformat_text_filter;

//! Represents precompiled executable title-formatting script. Use titleformat_compiler to instantiate; do not reimplement.
class NOVTABLE titleformat_object : public service_base
{
public:
	virtual void run(titleformat_hook * p_source,pfc::string_base & p_out,titleformat_text_filter * p_filter)=0;

	void run_hook(const playable_location & p_location,const file_info * p_source,titleformat_hook * p_hook,pfc::string_base & p_out,titleformat_text_filter * p_filter);
	void run_simple(const playable_location & p_location,const file_info * p_source,pfc::string_base & p_out);

	//! Helper, see titleformat_object_v2::requires_metadb_info()
	bool requires_metadb_info_();

	FB2K_MAKE_SERVICE_INTERFACE(titleformat_object,service_base);
};
