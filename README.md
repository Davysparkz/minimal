# minimal - an opinionated DirectX 12 framework
___
## Motivation
Usually the barrier to entry when trying to learn DirectX 12 is a cumbersome one.
This is due to the way in which the API was presented, such as ALL-CAPS declaration of
Interfaces, Structs and Enums, unusually long declaration names to name but a few.
These declarations in their own right bring a lot a visual clutter to the code.
As a means to tackle this issue, `minimal` was created.

`minimal` has namespaces such as `w32::` and `dx::` containing declarations all in SMALL-CAPS.

Take for example declarations in `w32::` would look like this:
```c++
UINT -> uint_t
HINSTANCE -> hinstance_t
WPARAM -> wparam_t
LPARAM -> lparam_t
```

Also, declarations in `dx::` would look like this:
```c++
ID3D12VersionedRootSignatureDeserializer -> versionedrootsignaturedeserializer_t
ID3D12GraphicsCommandList -> graphicscommandlist_t
ID3D12Device -> device_t
ID3D12DescriptorHeap -> descriptorheap_t
```

Types which are **interfaces** have a suffix of `_t` , **structs** have a suffix of `_s`, and **enums** have a suffix of `_e`.

The famous `IID_PPV_ARGS` is now:
```c++
iid_ppv_args
```

## Notes
Obviously, all types in Windows API and DirectX12 API cannot be represented in this framework. But overtime, most of the API will be featured in `minimal`