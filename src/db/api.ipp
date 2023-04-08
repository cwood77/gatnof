Char::Char(iDict& d, sst::dict& overlay)
: hp(100)
, m_overlay(overlay)
, m_pStatic(NULL)
{
   m_pStatic = &d.findChar(overlay["type"].as<sst::mint>().get());
}
