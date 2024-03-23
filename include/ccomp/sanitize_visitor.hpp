#ifndef CCOMP_SANITIZE_VISITOR_HPP
#define CCOMP_SANITIZE_VISITOR_HPP


#include <ccomp/ast_visitor.hpp>


namespace ccomp::ast
{
	struct sanitize_visitor : base_visitor
	{
		void visit(const procedure_statement&) override;
		void visit(const instruction_statement&) override;
		void visit(const define_statement &) override;
		void visit(const raw_statement&) override;
		void visit(const label_statement&) override;
	};
}


#endif //CCOMP_SANITIZE_VISITOR_HPP
