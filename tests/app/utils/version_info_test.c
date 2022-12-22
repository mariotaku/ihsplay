#include "util/version_info.h"

#include <assert.h>

int main() {
    version_info_t version;

    assert(version_info_parse(&version, "5") == 0);
    assert(version.major == 5 && version.minor == -1 && version.patch == -1);

    assert(version_info_parse(&version, "5.4") == 0);
    assert(version.major == 5 && version.minor == 4 && version.patch == -1);

    assert(version_info_parse(&version, "5.4.3") == 0);
    assert(version.major == 5 && version.minor == 4 && version.patch == 3);

    assert(version_info_parse(&version, "5.4.3.2") == 0);
    assert(version.major == 5 && version.minor == 4 && version.patch == 3);

    assert(version_info_parse(&version, "========5.4.3.2") != 0);

    assert(version_info_parse(&version, "    5.4.3") == 0);
    assert(version.major == 5 && version.minor == 4 && version.patch == 3);

    version_constraint_t constraint;

    assert(version_constraint_parse(&constraint, ">1.0.0") == 0);
    assert(constraint.operand == VERSION_GREATER_THAN);
    assert(constraint.version.major == 1 && constraint.version.minor == 0 && constraint.version.patch == 0);

    assert(version_constraint_parse(&constraint, "<=1.0.0") == 0);
    assert(constraint.operand == VERSION_LESSER_EQUALS);
    assert(constraint.version.major == 1 && constraint.version.minor == 0 && constraint.version.patch == 0);

    assert(version_constraint_parse(&constraint, "=   5") == 0);
    assert(constraint.operand == VERSION_EQUALS);
    assert(constraint.version.major == 5 && constraint.version.minor == -1 && constraint.version.patch == -1);

    assert(version_constraint_parse(&constraint, "5") == 0);
    assert(constraint.operand == VERSION_EQUALS);
    assert(constraint.version.major == 5 && constraint.version.minor == -1 && constraint.version.patch == -1);

    assert(version_constraint_parse(&constraint, "=5") == 0);
    version_info_parse(&version, "5.4.3");
    assert(version_constraint_check(&constraint, &version));

    assert(version_constraint_parse(&constraint, "<=5") == 0);
    version_info_parse(&version, "5.4.3");
    assert(version_constraint_check(&constraint, &version));
    version_info_parse(&version, "6.0.0");
    assert(!version_constraint_check(&constraint, &version));

    assert(version_constraint_parse(&constraint, "<>5") == 0);
    version_info_parse(&version, "6.0.0");
    assert(version_constraint_check(&constraint, &version));
    version_info_parse(&version, "5.3.1");
    assert(!version_constraint_check(&constraint, &version));

    assert(version_constraint_parse(&constraint, ">5") == 0);
    version_info_parse(&version, "6.0.0");
    assert(version_constraint_check(&constraint, &version));
    version_info_parse(&version, "4.9.9");
    assert(!version_constraint_check(&constraint, &version));
    return 0;
}