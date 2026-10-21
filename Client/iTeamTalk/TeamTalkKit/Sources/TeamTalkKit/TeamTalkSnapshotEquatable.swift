import Foundation

/// Byte-for-byte equality for a raw TeamTalk C struct (`User`, `Channel`, ...).
/// These structs only contain fixed-size character arrays, integers, floats and
/// bitmasks — no pointers — so their memory layout is a faithful representation
/// of their value, and this is more robust than hand-listing every field (which
/// would silently miss fields added to the SDK header later).
func rawStructsEqual<T>(_ lhs: T, _ rhs: T) -> Bool {
    withUnsafeBytes(of: lhs) { lhsBytes in
        withUnsafeBytes(of: rhs) { rhsBytes in
            lhsBytes.elementsEqual(rhsBytes)
        }
    }
}

/// Hashes a raw TeamTalk C struct by its raw bytes, consistent with `rawStructsEqual`.
func hashRawStruct<T>(_ value: T, into hasher: inout Hasher) {
    withUnsafeBytes(of: value) { hasher.combine(bytes: $0) }
}
