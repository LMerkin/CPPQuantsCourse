// vimn:ts=2:et
//===========================================================================//
//                                "Echo.hpp":                                //
//                      Elementary "Echo" HTTP Service                       //
//===========================================================================//
#pragma  once
#include <utxx/compiler_hints.hpp>
#include <utility>

namespace Net
{
	//=========================================================================//
	// "Echo" Class:                                                           //
	//=========================================================================//
	class Echo
	{
	private:
		constexpr static size_t BuffSz = 65536;
		char*	const	            m_buff;

	public:
		//=======================================================================//
		// Ctors, Dtor:																													 //
		//=======================================================================//
		// Default Ctor:
		Echo()
		:	m_buff(new char[BuffSz])
		{}

		// Copy ctor: In this case, it is the same as the Default Ctor:
		Echo(Echo const&)
		:	m_buff(new char[BuffSz])
		{}

		// Dtor:
		~Echo()
		{
			delete[] m_buff;
			const_cast<char*&>(m_buff) = nullptr; // For extra safety
		}

	private:
		//=======================================================================//
		// "Put" (for buffer safety):                                            //
		//=======================================================================//
		// Returns the ptr to the end of the string put, and the remaining buff sz:
		//
		static char* Put(char* a_out, char const* a_src, size_t& a_rem)
		{
			assert(a_out != nullptr && a_src != nullptr);
			if (UNLIKELY(a_rem <= 0))
				throw utxx::runtime_error("Buffer OverFlow");

			char*  end = stpncpy(a_out, a_src, a_rem);
			assert(end != nullptr  && end <= a_out);

			size_t n  = size_t (end - a_out);
			assert(n <= a_rem);
			if (UNLIKELY(n == a_rem))
				throw utxx::runtime_error("Ouutput Truncated");

			// All is fine:
			a_rem -= n;
			return end;
		}

	public:
		//=======================================================================//
		// Call-Back: To be unvoked as "UserAction" from "HTTPProtoDialogue":    //
		//=======================================================================//
		std::pair<char const*, size_t> operator()
		(
			char const* 															a_opname,
			int																				a_nargs,
			std::pair<char const*, char const*> const a_args[]
		)
		{
			assert(a_opname != nullptr);

			// Output the OpName and Params in HTML format.
			char*  out = m_buff;
			size_t rem = BuffSz;

			out = Put(out, "<html><title>",  rem);
			out = Put(out, a_opname,         rem);
			out = Put(out, "</title><body>", rem);
			for (int i = 0;  i < a_nargs;  ++i)
			{
				auto const& argi = a_args[i];		 // NB: const& here to avoid copying!
				char const* key  = argi.first;
				char const* val  = argi.second;

				// Modern C++ also allows:
				// auto [key,val]   = a_args[i];

				assert(key != nullptr && val != nullptr);
				out = Put(out, "<p>",   rem);
				out = Put(out, key,     rem);
				out = Put(out, " --> ", rem);
				out = Put(out, val,     rem);
				out = Put(out, "</p>",  rem);
			}
			out = Put(out, "</body></html>", rem);
			assert(out > m_buff);

			return std::make_pair(m_buff, size_t(out - m_buff)); // To avoid strlen()
		}
	};
}
// End namespace Net
