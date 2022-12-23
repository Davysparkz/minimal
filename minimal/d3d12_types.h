namespace dx {

#include <d3d12.h>

#define iid_ppv_args(ppType) IID_PPV_ARGS(ppType)


	// INTERFACES end with *_t
	using object_t = ID3D12Object;	
	using devicechild_t = ID3D12DeviceChild;
	using rootsignature_t = ID3D12RootSignature;
	using rootsignaturedeserializer_t = ID3D12RootSignatureDeserializer;
	using versionedrootsignaturedeserializer_t = ID3D12VersionedRootSignatureDeserializer;
    using pageable_t = ID3D12Pageable;
	using heap_t = ID3D12Heap;
	using commandallocator_t = ID3D12CommandAllocator;
	using fence_t = ID3D12Fence;
	using factory4_t = IDXGIFactory4;
	using factory5_t = IDXGIFactory5;
	using pipeline_state_t = ID3D12PipelineState;
	using descriptorheap_t = ID3D12DescriptorHeap;
	using queryheap_t = ID3D12QueryHeap;
	using commandsignature_t = ID3D12CommandSignature;
	using commandlist_t = ID3D12CommandList;
	using commandqueue_t = ID3D12CommandQueue;
	using commandallocator_t = ID3D12CommandAllocator;
	using adapter1_t = IDXGIAdapter1;
	using adapter4_t = IDXGIAdapter4;
	using debug_t = ID3D12Debug;
	using infoqueue_t = ID3D12InfoQueue;
	using resourcebarrier_t = CD3DX12_RESOURCE_BARRIER;
	using cpudescriptorhandle_t = CD3DX12_CPU_DESCRIPTOR_HANDLE;
	using graphicscommandlist_t = ID3D12GraphicsCommandList;
	using graphicscommandlist1_t = ID3D12GraphicsCommandList1;
	using graphicscommandlist2_t = ID3D12GraphicsCommandList2;
	using commandqueue_t = ID3D12CommandQueue;
	using pipelinelibrary_t = ID3D12PipelineLibrary;
	using device_t = ID3D12Device;
	using device1_t = ID3D12Device1;
	using device2_t = ID3D12Device2;
	using device3_t = ID3D12Device3;
	using device4_t = ID3D12Device4;
	using device5_t = ID3D12Device5;
	using device6_t = ID3D12Device6;
	using device7_t = ID3D12Device7;
	using device8_t = ID3D12Device8;
	using protectedsession_t = ID3D12ProtectedSession;
	using protectedresourcesession_t = ID3D12ProtectedResourceSession;
	using lifetimeowner_t = ID3D12LifetimeOwner;
	using lifetimetracker_t = ID3D12LifetimeTracker;
	using swapchain1_t = IDXGISwapChain1;
	using swapchain4_t = IDXGISwapChain4;
	using swapchainassistant_t = ID3D12SwapChainAssistant;
	using stateobject_t = ID3D12StateObject;
	using stateobjectproperties_t = ID3D12StateObjectProperties;
	using deviceremovedextendeddatasettings_t = ID3D12DeviceRemovedExtendedDataSettings;
	using deviceremovedextendeddatasettings1_t = ID3D12DeviceRemovedExtendedDataSettings1;
	using deviceremovedextendeddata_t = ID3D12DeviceRemovedExtendedData;
	using deviceremovedextendeddata1_t = ID3D12DeviceRemovedExtendedData1;
	using protectedresourcesession1_t = ID3D12ProtectedResourceSession1;
	using resource_t = ID3D12Resource;
	using resource1_t = ID3D12Resource1;
	using resource2_t = ID3D12Resource2;
	using heap_t = ID3D12Heap;
	using graphicscommandlist3_t = ID3D12GraphicsCommandList3;
	using metacommand_t = ID3D12MetaCommand;
	using graphicscommandlist4_t = ID3D12GraphicsCommandList4;
	using tools_t = ID3D12Tools;
	using graphicscommandlist5_t = ID3D12GraphicsCommandList5;
	using graphicscommandlist6_t = ID3D12GraphicsCommandList6;


	// STRUCTS end with *_s
	using dxgiswapchaindesc_s = DXGI_SWAP_CHAIN_DESC;
	using dxgiswapchaindesc1_s = DXGI_SWAP_CHAIN_DESC1;
	using commandqueuedesc_s = D3D12_COMMAND_QUEUE_DESC;
	using infoqueuefilter_s = D3D12_INFO_QUEUE_FILTER;
	using message_severity_s = D3D12_MESSAGE_SEVERITY;
	using dxgiadapterdesc1_s = DXGI_ADAPTER_DESC1;

	// ENUMS end with *_e
	using commandlisttype_e = D3D12_COMMAND_LIST_TYPE;
	using messageid_e = D3D12_MESSAGE_ID;
}
