#include "foobar2000-sdk-pch.h"
#include "dsp.h"
#include "resampler.h"

#ifdef FOOBAR2000_HAVE_DSP

#include <math.h>

audio_chunk * dsp_chunk_list::add_item(t_size hint_size) { return insert_item(get_count(), hint_size); }

void dsp_chunk_list::remove_all() { remove_mask(pfc::bit_array_true()); }

double dsp_chunk_list::get_duration() {
	double rv = 0;
	t_size n, m = get_count();
	for (n = 0; n<m; n++) rv += get_item(n)->get_duration();
	return rv;
}

void dsp_chunk_list::add_chunk(const audio_chunk * chunk) {
	audio_chunk * dst = insert_item(get_count(), chunk->get_used_size());
	if (dst) dst->copy(*chunk);
}

t_size dsp_chunk_list_impl::get_count() const {return m_data.size();}

audio_chunk * dsp_chunk_list_impl::get_item(t_size n) const {return n<m_data.size() ? m_data[n].get() : 0; }

void dsp_chunk_list_impl::remove_by_idx(t_size idx)
{
	PFC_ASSERT(idx < get_count());
	m_recycled.push_back(std::move(m_data[idx]));
	m_data.erase(m_data.begin() + idx);
}

void dsp_chunk_list_impl::remove_mask(const bit_array & mask)
{
	const auto total = m_data.size();
	mask.for_each(true, 0, total, [&](size_t idx) {
		m_recycled.push_back(std::move(m_data[idx]));
	});
	pfc::remove_mask_t(m_data, mask);
}

audio_chunk * dsp_chunk_list_impl::insert_item(t_size idx,t_size hint_size)
{
	t_size max = get_count();
	if (idx>max) idx = max;
	chunk_ptr_t ret;
	if (!m_recycled.empty())
	{
		t_size best;
		if (hint_size > 0)
		{
			best = 0;
			t_size best_found = m_recycled[0]->get_data_size(), n, total = m_recycled.size();
			for (n = 1; n < total; n++)
			{
				if (best_found == hint_size) break;
				t_size size = m_recycled[n]->get_data_size();
				int delta_old = abs((int)best_found - (int)hint_size), delta_new = abs((int)size - (int)hint_size);
				if (delta_new < delta_old)
				{
					best_found = size;
					best = n;
				}
			}
		} else best = m_recycled.size() - 1;

		ret = std::move(m_recycled[best]);
		m_recycled.erase(m_recycled.begin() + best);
		ret->set_sample_count(0);
		ret->set_channels(0);
		ret->set_srate(0);
	} else ret = std::make_unique<audio_chunk_impl>();
	auto pRet = &*ret;
	if (idx == max) m_data.push_back(std::move(ret));
	else m_data.insert(m_data.begin() + idx, std::move(ret));
	return pRet;
}

void dsp_chunk_list::remove_bad_chunks()
{
	bool blah = false;
	t_size idx;
	for(idx=0;idx<get_count();)
	{
		audio_chunk * chunk = get_item(idx);
		if (!chunk->is_valid())
		{
#if PFC_DEBUG
			FB2K_console_formatter() << "Removing bad chunk: " << chunk->formatChunkSpec();
#endif
			chunk->reset();
			remove_by_idx(idx);
			blah = true;
		}
		else idx++;
	}
	if (blah) console::info("one or more bad chunks removed from dsp chunk list");
}

bool dsp_entry_hidden::g_dsp_exists(const GUID & p_guid) {
	dsp_entry_hidden::ptr p;
	return g_get_interface(p, p_guid);
}

bool dsp_entry_hidden::g_get_interface( dsp_entry_hidden::ptr & out, const GUID & guid ) {
	for (auto p : enumerate()) {
		if (p->get_guid() == guid) {
			out = p; return true;
		}
	}
	return false;
}

bool dsp_entry_hidden::g_instantiate( dsp::ptr & out, const dsp_preset & preset ) {
	dsp_entry_hidden::ptr i;
	if (!g_get_interface(i, preset.get_owner())) return false;
	return i->instantiate(out, preset);
}

bool dsp_entry::g_instantiate(service_ptr_t<dsp> & p_out,const dsp_preset & p_preset, unsigned flags )
{
	service_ptr_t<dsp_entry> ptr;
	if (!g_get_interface(ptr,p_preset.get_owner())) return false;
    if ( flags != 0 ) {
        dsp_entry_v4::ptr v4;
        if (v4 &= ptr) {
            p_out = v4->instantiate_v4(p_preset, flags);
            return true;
        }
    }
	return ptr->instantiate(p_out,p_preset);
}

bool dsp_entry::g_instantiate_default(service_ptr_t<dsp> & p_out,const GUID & p_guid)
{
	service_ptr_t<dsp_entry> ptr;
	if (!g_get_interface(ptr,p_guid)) return false;
	dsp_preset_impl preset;
	if (!ptr->get_default_preset(preset)) return false;
	return ptr->instantiate(p_out,preset);
}

bool dsp_entry::g_name_from_guid(pfc::string_base & p_out,const GUID & p_guid)
{
	service_ptr_t<dsp_entry> ptr;
	if (!g_get_interface(ptr,p_guid)) return false;
	ptr->get_name(p_out);
	return true;
}

bool dsp_entry::g_dsp_exists(const GUID & p_guid)
{
	service_ptr_t<dsp_entry> blah;
	return g_get_interface(blah,p_guid);
}

bool dsp_entry::g_get_default_preset(dsp_preset & p_out,const GUID & p_guid)
{
	service_ptr_t<dsp_entry> ptr;
	if (!g_get_interface(ptr,p_guid)) return false;
	return ptr->get_default_preset(p_out);
}

void dsp_chain_config::contents_to_stream(stream_writer * p_stream,abort_callback & p_abort) const {
    uint32_t n, count = pfc::downcast_guarded<uint32_t>( get_count() );
	p_stream->write_lendian_t(count,p_abort);
	for(n=0;n<count;n++) {
		get_item(n).contents_to_stream(p_stream,p_abort);
	}
}

fb2k::memBlock::ptr dsp_chain_config::to_blob() const {
	stream_writer_buffer_simple out;
	this->contents_to_stream(&out, fb2k::noAbort);
	return fb2k::memBlock::blockWithVector(out.m_buffer);
}

void dsp_chain_config::from_blob(const void* p, size_t size) {
	if (size == 0) {
		remove_all(); return;
	}
	stream_reader_memblock_ref reader(p, size);
	this->contents_from_stream(&reader, fb2k::noAbort);
}

void dsp_chain_config::from_blob(fb2k::memBlock::ptr b) {
	if (b.is_valid()) {
		from_blob(b->data(), b->size());
	} else {
		this->remove_all();
	}
}

void dsp_chain_config::contents_from_stream(stream_reader * p_stream,abort_callback & p_abort) {
	t_uint32 n,count;

	remove_all();

	p_stream->read_lendian_t(count,p_abort);

	dsp_preset_impl temp;

	for(n=0;n<count;n++) {
		temp.contents_from_stream(p_stream,p_abort);
		add_item(temp);
	}
}

void dsp_chain_config::remove_item(t_size p_index)
{
	remove_mask(pfc::bit_array_one(p_index));
}

void dsp_chain_config::add_item(const dsp_preset & p_data)
{
	insert_item(p_data,get_count());
}

void dsp_chain_config::remove_all()
{
	remove_mask(pfc::bit_array_true());
}

size_t dsp_chain_config::find_first_of_type( const GUID & dspID ) const {
    const size_t count = this->get_count();
    for(size_t w = 0; w < count; ++w) {
        if (this->get_item(w).get_owner() == dspID) return w;
    }
    return pfc_infinite;
}

bool dsp_chain_config::contains_dsp( const GUID & dspID ) const {
    return find_first_of_type( dspID ) != pfc_infinite;
}

bool dsp_chain_config::enable_dsp( const GUID & dspID ) {
    if (this->contains_dsp( dspID )) return false;
    dsp_preset_impl preset;
    dsp_entry::g_get_default_preset( preset, dspID );
    insert_item( preset, 0 );
    return true;
}

bool dsp_chain_config::disable_dsp( const GUID & dspID ) {
    const size_t count = this->get_count();
    if (count == 0) return false;
    bool rv = false;
	pfc::bit_array_bittable mask( count );
    for(size_t w = 0; w < count; ++ w) {
        if (this->get_item(w).get_owner() == dspID ) {
            rv = true;
            mask.set(w, true);
        }
    }
    if (rv) this->remove_mask( mask );
    return rv;
}

bool dsp_chain_config::enable_dsp( const dsp_preset & preset ) {
    dsp_chain_config & cfg = *this;
    bool found = false;
    bool changed = false;
    t_size n,m = cfg.get_count();
    for(n=0;n<m;n++) {
        if (cfg.get_item(n).get_owner() == preset.get_owner()) {
            found = true;
            if (cfg.get_item(n) != preset) {
                cfg.replace_item(preset,n);
                changed = true;
            }
            break;
        }
    }
    if (!found) {cfg.insert_item(preset,0); changed = true;}
    
    return changed;
}

void dsp_chain_config_impl::reorder(const size_t * order, size_t count) {
	PFC_ASSERT( count == m_data.get_count() );
	m_data.reorder( order );
}

t_size dsp_chain_config_impl::get_count() const
{
	return m_data.get_count();
}

const dsp_preset & dsp_chain_config_impl::get_item(t_size p_index) const
{
	return m_data[p_index]->data;
}

void dsp_chain_config_impl::replace_item(const dsp_preset & p_data,t_size p_index)
{
	auto& obj = *m_data[p_index];
	if (p_data.get_owner() != obj.data.get_owner()) {
		obj.dspName = p_data.get_owner_name();
	}
	obj.data = p_data;
}

void dsp_chain_config_impl::insert_item(const dsp_preset & p_data,t_size p_index)
{
	this->insert_item_v2(p_data, nullptr, p_index);
}

void dsp_chain_config_impl::remove_mask(const bit_array & p_mask)
{
	m_data.delete_mask(p_mask);
}

dsp_chain_config_impl::~dsp_chain_config_impl()
{
	m_data.delete_all();
}

const char* dsp_chain_config_impl::get_dsp_name(size_t idx) const {
	auto& n = m_data[idx]->dspName;
	if (n.is_empty()) return nullptr;
	return n.c_str();
}

void dsp_chain_config_impl::insert_item_v2(const dsp_preset& data, const char* dspName_, size_t index) {
	pfc::string8 dspName;
	if (dspName_) dspName = dspName_;
	if (dspName.length() == 0) dspName = data.get_owner_name();
	m_data.insert_item(new entry_t{ data, std::move(dspName) }, index);
}

const char* dsp_chain_config_impl::find_dsp_name(const GUID& guid) const {
	for (size_t walk = 0; walk < m_data.get_size(); ++walk) {
		auto& obj = *m_data[walk];
		if (obj.data.get_owner() == guid && obj.dspName.length() > 0) {
			return obj.dspName.c_str();
		}
	}
	return nullptr;
}

pfc::string8 dsp_preset::get_owner_name() const {
	pfc::string8 ret;
	dsp_entry::ptr obj;
	if (dsp_entry::g_get_interface(obj, this->get_owner())) {
		obj->get_name(ret);
	}
	return ret;
}

pfc::string8 dsp_preset::get_owner_name_debug() const {
	pfc::string8 ret;
	dsp_entry::ptr obj;
	if (dsp_entry::g_get_interface(obj, this->get_owner())) {
		obj->get_name(ret);
	} else {
		ret = "[unknown]";
	}
	return ret;
}

pfc::string8 dsp_preset::debug(const char * knownName) const {
	pfc::string8 name;
	if (knownName) name = knownName;
	else name = this->get_owner_name_debug();
	pfc::string8 ret;
	ret << name << " :: " << pfc::print_guid(this->get_owner()) << " :: " << pfc::format_hexdump(this->get_data(), this->get_data_size());
	return ret;
}

pfc::string8 dsp_chain_config::debug() const {
	const size_t count = get_count();
	pfc::string8 ret;
	ret << "dsp_chain_config: " << count << " items";
	for (size_t walk = 0; walk < count; ++walk) {
		ret << "\n" << get_item(walk).debug();
	}
	return ret;	
}

void dsp_chain_config_impl::add_item_v2(const dsp_preset& data, const char* dspName) {
	insert_item_v2(data, dspName, get_count());
}

void dsp_chain_config_impl::copy_v2(dsp_chain_config_impl const& p_source) {
	remove_all();
	t_size n, m = p_source.get_count();
	for (n = 0; n < m; n++)
		add_item_v2(p_source.get_item(n), p_source.get_dsp_name(n));
}

pfc::string8 dsp_chain_config_impl::debug() const {
	const size_t count = get_count();
	pfc::string8 ret;
	ret << "dsp_chain_config_impl: " << count << " items";
	for (size_t walk = 0; walk < count; ++walk) {
		ret << "\n" << get_item(walk).debug( this->get_dsp_name(walk) );
	}
	return ret;
}

void dsp_preset::contents_to_stream(stream_writer * p_stream,abort_callback & p_abort) const {
    t_uint32 size = pfc::downcast_guarded<t_uint32>(get_data_size());
	p_stream->write_lendian_t(get_owner(),p_abort);
	p_stream->write_lendian_t(size,p_abort);
	if (size > 0) {
		p_stream->write_object(get_data(),size,p_abort);
	}
}

void dsp_preset::contents_from_stream(stream_reader * p_stream,abort_callback & p_abort) {
	t_uint32 size;
	GUID guid;
	p_stream->read_lendian_t(guid,p_abort);
	set_owner(guid);
	p_stream->read_lendian_t(size,p_abort);
	if (size > 1024*1024*32) throw exception_io_data();
	set_data_from_stream(p_stream,size,p_abort);
}

void dsp_preset::g_contents_from_stream_skip(stream_reader * p_stream,abort_callback & p_abort) {
	t_uint32 size;
	GUID guid;
	p_stream->read_lendian_t(guid,p_abort);
	p_stream->read_lendian_t(size,p_abort);
	if (size > 1024*1024*32) throw exception_io_data();
	p_stream->skip_object(size,p_abort);
}

void dsp_preset_impl::set_data_from_stream(stream_reader * p_stream,t_size p_bytes,abort_callback & p_abort) {
	m_data.resize(p_bytes);
	if (p_bytes > 0) p_stream->read_object(m_data.ptr(),p_bytes,p_abort);
}

void dsp_chain_config::copy(const dsp_chain_config & p_source) {
	remove_all();
	t_size n, m = p_source.get_count();
	for(n=0;n<m;n++)
		add_item(p_source.get_item(n));
}

bool dsp_entry::g_have_config_popup(const GUID & p_guid)
{
	service_ptr_t<dsp_entry> entry;
	if (!g_get_interface(entry,p_guid)) return false;
	return entry->have_config_popup();
}

bool dsp_entry::g_have_config_popup(const dsp_preset & p_preset)
{
	return g_have_config_popup(p_preset.get_owner());
}

#ifdef _WIN32
bool dsp_entry::g_show_config_popup(dsp_preset & p_preset,fb2k::hwnd_t p_parent)
{
	service_ptr_t<dsp_entry> entry;
	if (!g_get_interface(entry,p_preset.get_owner())) return false;
	return entry->show_config_popup(p_preset,p_parent);
}

bool dsp_entry::show_config_popup_v2_(const dsp_preset& p_preset, fb2k::hwnd_t p_parent, dsp_preset_edit_callback& p_callback) {
	PFC_ASSERT(p_preset.get_owner() == this->get_guid());
	try {
		service_ptr_t<dsp_entry_v2> entry_v2;
		if (entry_v2 &= this) {
			entry_v2->show_config_popup_v2(p_preset, p_parent, p_callback);
			return true;
		}
	} catch (pfc::exception_not_implemented const&) {}

	dsp_preset_impl temp(p_preset);
	bool rv = this->show_config_popup(temp, p_parent);
	if (rv) p_callback.on_preset_changed(temp);
	return rv;
}
namespace {
	class dsp_preset_edit_callback_callV2 : public dsp_preset_edit_callback {
	public:
		dsp_preset_edit_callback_v2::ptr chain;
		void on_preset_changed(const dsp_preset& arg) override { chain->set_preset(arg); }
	};
}
service_ptr dsp_entry::show_config_popup_v3_(fb2k::hwnd_t parent, dsp_preset_edit_callback_v2::ptr callback) {
	dsp_entry_v3::ptr v3;
	if (v3 &= this) {
		try {
			return v3->show_config_popup_v3(parent, callback);
		} catch (pfc::exception_not_implemented const &) {
		}
	}

	dsp_preset_edit_callback_callV2 cb;
	cb.chain = callback;

	dsp_preset_impl initPreset; callback->get_preset(initPreset);
	bool status = this->show_config_popup_v2_(initPreset, parent, cb);
	callback->dsp_dialog_done(status);
	return nullptr;

}
void dsp_entry::g_show_config_popup_v2(const dsp_preset & p_preset,fb2k::hwnd_t p_parent,dsp_preset_edit_callback & p_callback) {
	auto api = g_get_interface(p_preset.get_owner());
	if (api.is_valid()) api->show_config_popup_v2_(p_preset, p_parent, p_callback);
}
#endif

service_ptr_t<dsp_entry> dsp_entry::g_get_interface(const GUID& guid) {
	for (auto ptr : enumerate()) {
		if (ptr->get_guid() == guid) return ptr;
	}
	return nullptr;
}

bool dsp_entry::g_get_interface(service_ptr_t<dsp_entry> & p_out,const GUID & p_guid)
{
	for (auto ptr : enumerate()) {
		if (ptr->get_guid() == p_guid) {
			p_out = ptr;
			return true;
		}
	}
	return false;
}

bool resampler_entry::g_get_interface(service_ptr_t<resampler_entry> & p_out,unsigned p_srate_from,unsigned p_srate_to)
{
#if defined(FOOBAR2000_DESKTOP) && FOOBAR2000_TARGET_VERSION >= 79
	auto r = resampler_manager::get()->get_resampler( p_srate_from, p_srate_to );
	bool v = r.is_valid();
	if ( v ) p_out = std::move(r);
	return v;
#else
    
#ifdef FOOBAR2000_DESKTOP
	{
		resampler_manager::ptr api;
		if ( resampler_manager::tryGet(api) ) {
			auto r = api->get_resampler( p_srate_from, p_srate_to );
			bool v = r.is_valid();
			if (v) p_out = std::move(r);
			return v;
		}
	}
#endif

	resampler_entry::ptr ptr_resampler;
	service_enum_t<dsp_entry> e;
	float found_priority = 0;
	resampler_entry::ptr found;
	while(e.next(ptr_resampler))
	{
		if (p_srate_from == 0 || ptr_resampler->is_conversion_supported(p_srate_from,p_srate_to))
		{
			float priority = ptr_resampler->get_priority();
			if (found.is_empty() || priority > found_priority)
			{
				found = ptr_resampler;
				found_priority = priority;
			}
		}
	}
	if (found.is_empty()) return false;
	p_out = found;
	return true;
#endif
}

bool resampler_entry::g_create_preset(dsp_preset & p_out,unsigned p_srate_from,unsigned p_srate_to,float p_qualityscale)
{
	service_ptr_t<resampler_entry> entry;
	if (!g_get_interface(entry,p_srate_from,p_srate_to)) return false;
	return entry->create_preset(p_out,p_srate_to,p_qualityscale);
}

bool resampler_entry::g_create(service_ptr_t<dsp> & p_out,unsigned p_srate_from,unsigned p_srate_to,float p_qualityscale)
{
	service_ptr_t<resampler_entry> entry;
	if (!g_get_interface(entry,p_srate_from,p_srate_to)) return false;
	dsp_preset_impl preset;
	if (!entry->create_preset(preset,p_srate_to,p_qualityscale)) return false;
	return entry->instantiate(p_out,preset);
}


bool dsp_chain_config::equals(dsp_chain_config const & v1, dsp_chain_config const & v2) {
	const t_size count = v1.get_count();
	if (count != v2.get_count()) return false;
	for(t_size walk = 0; walk < count; ++walk) {
		if (v1.get_item(walk) != v2.get_item(walk)) return false;
	}
	return true;
}
bool dsp_chain_config::equals_debug(dsp_chain_config const& v1, dsp_chain_config const& v2) {
	FB2K_DebugLog() << "Comparing DSP chains";
	const t_size count = v1.get_count();
	if (count != v2.get_count()) {
		FB2K_DebugLog() << "Count mismatch, " << count << " vs " << v2.get_count();
		return false;
	}
	for (t_size walk = 0; walk < count; ++walk) {
		if (v1.get_item(walk) != v2.get_item(walk)) {
			FB2K_DebugLog() << "Item " << (walk+1) << " mismatch";
			FB2K_DebugLog() << "Item 1: " << v1.get_item(walk).debug();
			FB2K_DebugLog() << "Item 2: " << v2.get_item(walk).debug();
			return false;
		}
	}
	FB2K_DebugLog() << "DSP chains are identical";
	return true;
}

void dsp_chain_config::get_name_list(pfc::string_base & p_out) const {
	p_out = get_name_list();
}

pfc::string8 dsp_chain_config::get_name_list() const {
	const size_t count = get_count();
	pfc::string8 output; output.prealloc(1024);
	for (size_t n = 0; n < count; n++)
	{
		const auto& preset = get_item(n);
		service_ptr_t<dsp_entry> ptr;
		if (dsp_entry::g_get_interface(ptr, preset.get_owner()))
		{
			pfc::string8 temp;
			ptr->get_display_name_(preset, temp);
			if (temp.length() > 0) {
				if (output.length() > 0) output += ", ";
				output += temp;
			}
		}
	}

	return output;
}

void dsp::run_abortable(dsp_chunk_list * p_chunk_list,const dsp_track_t & p_cur_file,int p_flags,abort_callback & p_abort) {
	service_ptr_t<dsp_v2> this_v2;
	if (this->service_query_t(this_v2)) this_v2->run_v2(p_chunk_list,p_cur_file,p_flags,p_abort);
	else run(p_chunk_list,p_cur_file,p_flags);
}

bool dsp::apply_preset_(const dsp_preset& arg) {
	dsp_v3::ptr v3;
	if (v3 &= this) return v3->apply_preset(arg);
	return false;
}

namespace {
	class dsp_preset_edit_callback_impl : public dsp_preset_edit_callback {
	public:
		dsp_preset_edit_callback_impl(dsp_preset & p_data) : m_data(p_data) {}
		void on_preset_changed(const dsp_preset & p_data) {m_data = p_data;}
	private:
		dsp_preset & m_data;
	};
};

#ifdef _WIN32
bool dsp_entry_v2::show_config_popup(dsp_preset & p_data,fb2k::hwnd_t p_parent) {
	PFC_ASSERT(p_data.get_owner() == get_guid());
	dsp_preset_impl temp(p_data);
    
    {
        dsp_preset_edit_callback_impl cb(temp);
        show_config_popup_v2(p_data,p_parent,cb);
    }
	PFC_ASSERT(temp.get_owner() == get_guid());
	if (temp == p_data) return false;
	p_data = temp;
	return true;
}
#endif

#ifdef FOOBAR2000_MOBILE
void dsp_entry::g_show_config_popup( menu_context_ptr ctx, dsp_preset_edit_callback_v2::ptr callback) {
    GUID dspID;
    {
        dsp_preset_impl temp;
        callback->get_preset( temp );
        dspID = temp.get_owner();
    }

    dsp_entry::ptr entry;
    if (!g_get_interface( entry, dspID)) return;
    if (!entry->have_config_popup()) return;
    entry->show_config_popup( ctx, callback );
}
#endif // FOOBAR2000_MOBILE

#ifdef FOOBAR2000_DESKTOP
void resampler_manager::make_chain_(dsp_chain_config& outChain, unsigned rateFrom, unsigned rateTo, float qualityScale) {
	resampler_manager_v2::ptr v2;
	if (v2 &= this) {
		v2->make_chain(outChain, rateFrom, rateTo, qualityScale);
	} else {
		outChain.remove_all();
		auto obj = this->get_resampler(rateFrom, rateTo);
		if (obj.is_valid()) {
			dsp_preset_impl p;
			if (obj->create_preset(p, rateTo, qualityScale)) {
				outChain.add_item(p);
			}
		}
	}
}
#endif

void dsp_preset_edit_callback_v2::reset() {
    dsp_preset_impl temp; get_preset( temp );
    GUID id = temp.get_owner(); temp.set_data(nullptr, 0);
    if (dsp_entry::g_get_default_preset( temp, id )) {
        this->set_preset( temp );
    } else {
        PFC_ASSERT(!"Should not get here - no such DSP");
    }
}

bool dsp_entry::get_display_name_supported() {
	dsp_entry_v3::ptr v3;
	return v3 &= this;
}

void dsp_entry::get_display_name_(const dsp_preset& arg, pfc::string_base& out) {
	PFC_ASSERT(arg.get_owner() == this->get_guid());
	dsp_entry_v3::ptr v3;
	if (v3 &= this) {
		v3->get_display_name(arg, out); return;
	}
	get_name(out);
}

bool dsp_entry::enumerate_default_presets_(dsp_chain_config& ret) {
	ret.remove_all();
	dsp_entry_v5::ptr v5;
	if (v5 &= this) {
		bool rv = v5->enumerate_default_presets(ret);
#if PFC_DEBUG
		for (size_t walk = 0; walk < ret.get_count(); ++walk) {
			PFC_ASSERT(ret.get_item(walk).get_owner() == get_guid());
		}
#endif
		return rv;
	}
	return false;
}

bool dsp_entry::match_preset_subclass_(dsp_preset const& x, dsp_preset const& y) {
	dsp_entry_v5::ptr v5;
	if (v5 &= this) return v5->match_preset_subclass(x, y);
	return true;
}

#endif // FOOBAR2000_HAVE_DSP
