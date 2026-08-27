#!/usr/bin/env node
import assert from "node:assert/strict";
import fs from "node:fs";
import vm from "node:vm";
import {TextDecoder, TextEncoder} from "node:util";
import {fileURLToPath} from "node:url";
import path from "node:path";

const STATUS_SUCCESS = 1;
const STATUS_ERROR = 3;
const STATUS_REQUEST_ADAPTER_ERROR = 4;
const STATUS_VALIDATION_ERROR = 2;
const STATUS_UNKNOWN_ERROR = 5;
const CALLBACK_USERDATA1 = 0x11111111;
const CALLBACK_USERDATA2 = 0x22222222;

class FakeMemory {
	constructor(size = 1 << 20) {
		this.buffer = new ArrayBuffer(size);
		this.view = new DataView(this.buffer);
		this.bytes = new Uint8Array(this.buffer);
		this.intSize = 4;
		this.nextAlloc = 0x1000;
		this.callbacks = [];
		this.freed = [];
		this.allocations = new Map();
		this.freeList = [];
		this.exports = {
			wgpu_alloc: (size) => {
				const allocSize = (size + 7) & ~7;
				const freeIndex = this.freeList.findIndex((entry) => entry.size === allocSize);
				let ptr;
				if (freeIndex >= 0) {
					ptr = this.freeList.splice(freeIndex, 1)[0].ptr;
				} else {
					ptr = this.nextAlloc;
					this.nextAlloc += allocSize;
				}
				this.allocations.set(ptr, allocSize);
				return ptr;
			},
			wgpu_free: (ptr) => {
				this.freed.push(ptr);
				const size = this.allocations.get(ptr);
				if (size != null) {
					this.freeList.push({ptr, size});
					this.allocations.delete(ptr);
				}
			},
			__indirect_function_table: {
				get: (idx) => this.callbacks[idx - 1],
			},
		};
	}

	registerCallback(fn) {
		const idx = this.callbacks.length + 1;
		this.callbacks.push(fn);
		return idx;
	}

	loadPtr(addr) {
		return this.view.getUint32(addr, true);
	}

	loadUint(addr) {
		return this.view.getUint32(addr, true);
	}

	loadU32(addr) {
		return this.view.getUint32(addr, true);
	}

	loadB32(addr) {
		return this.view.getUint32(addr, true) !== 0;
	}

	storeI32(addr, value) {
		this.view.setInt32(addr, value | 0, true);
	}

	storeUint(addr, value) {
		if (typeof value === "bigint") {
			this.view.setBigUint64(addr, value, true);
			return;
		}

		this.view.setUint32(addr, value >>> 0, true);
	}

	storeU64(addr, value) {
		this.view.setBigUint64(addr, BigInt(value), true);
	}

	storeB32(addr, value) {
		this.view.setUint32(addr, value ? 1 : 0, true);
	}

	storeString(addr, value) {
		this.bytes.set(new TextEncoder().encode(value), addr);
	}

	loadString(addr, len) {
		return new TextDecoder().decode(this.bytes.subarray(addr, addr + Number(len)));
	}
}

function loadWebGPUInterface() {
	const sourcePath = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "wgpu.js");
	const source = fs.readFileSync(sourcePath, "utf8");
	const context = vm.createContext({
		Array,
		ArrayBuffer,
		BigInt,
		Boolean,
		DataView,
		Date,
		Error,
		JSON,
		Map,
		Math,
		Number,
		Object,
		Promise,
		RangeError,
		RegExp,
		Set,
		String,
		Symbol,
		TextDecoder,
		TextEncoder,
		TypeError,
		Uint8Array,
		Uint16Array,
		Uint32Array,
		Int8Array,
		Int16Array,
		Int32Array,
		BigInt64Array,
		BigUint64Array,
		console,
		queueMicrotask,
		setImmediate,
		clearImmediate,
		setTimeout,
		clearTimeout,
		window: {},
		navigator: {},
		GPUValidationError: class GPUValidationError extends Error {},
		GPUOutOfMemoryError: class GPUOutOfMemoryError extends Error {},
		GPUInternalError: class GPUInternalError extends Error {},
	});
	context.window = context;
	context.globalThis = context;
	vm.runInContext(source, context, {filename: sourcePath});
	return {
		WebGPUInterface: context.window.odin.WebGPUInterface,
		context,
	};
}

function writeCallbackInfo(mem, addr, callbackIdx, userdata1 = CALLBACK_USERDATA1, userdata2 = CALLBACK_USERDATA2) {
	mem.storeI32(addr + 0, 0);
	mem.storeI32(addr + 4, 0);
	mem.storeI32(addr + 8, callbackIdx);
	mem.storeI32(addr + 12, userdata1);
	mem.storeI32(addr + 16, userdata2);
	return addr;
}

function writeUncapturedErrorCallbackInfo(mem, addr, callbackIdx, userdata1 = CALLBACK_USERDATA1, userdata2 = CALLBACK_USERDATA2) {
	mem.storeI32(addr + 0, 0);
	mem.storeI32(addr + 4, callbackIdx);
	mem.storeI32(addr + 8, userdata1);
	mem.storeI32(addr + 12, userdata2);
	return addr;
}

function makeCallbackInfo(mem, handler, userdata1 = CALLBACK_USERDATA1, userdata2 = CALLBACK_USERDATA2) {
	const callbackIdx = mem.registerCallback(handler);
	const inner = mem.exports.wgpu_alloc(20);
	return writeCallbackInfo(mem, inner, callbackIdx, userdata1, userdata2);
}

function makeNullCallbackInfo(mem, userdata1 = CALLBACK_USERDATA1, userdata2 = CALLBACK_USERDATA2) {
	const inner = mem.exports.wgpu_alloc(20);
	return writeCallbackInfo(mem, inner, 0, userdata1, userdata2);
}

function makeRequestDeviceDescriptor(mem, options = {}) {
	const descriptor = mem.exports.wgpu_alloc(72);
	if (options.deviceLostHandler) {
		writeCallbackInfo(mem, descriptor + 36, mem.registerCallback(options.deviceLostHandler));
	}
	if (options.uncapturedErrorHandler) {
		writeUncapturedErrorCallbackInfo(mem, descriptor + 56, mem.registerCallback(options.uncapturedErrorHandler));
	}
	return descriptor;
}

function readStringView(mem, ptr) {
	return {
		dataPtr: mem.loadPtr(ptr),
		length: mem.loadUint(ptr + mem.intSize),
		text: mem.loadString(mem.loadPtr(ptr), mem.loadUint(ptr + mem.intSize)),
	};
}

function assertTerminalCallback(call, expectedArgc, expectedStatus) {
	assert.equal(call.length, expectedArgc, "wrong callback argc");
	assert.equal(call[0], expectedStatus, "wrong callback status");
	assert.equal(call[call.length - 2], CALLBACK_USERDATA1, "wrong userdata1");
	assert.equal(call[call.length - 1], CALLBACK_USERDATA2, "wrong userdata2");
}

async function settle() {
	await new Promise((resolve) => setImmediate(resolve));
	await new Promise((resolve) => setImmediate(resolve));
}

function makeInterface() {
	const mem = new FakeMemory();
	const {WebGPUInterface, context} = loadWebGPUInterface();
	const iface = new WebGPUInterface(mem);
	Object.assign(iface, iface.getInterface());
	return {mem, iface, context};
}

async function expectPromiseCallback(name, invoke, expectedStatus, expectedArgc, callbackReader, onCall) {
	const {mem, iface, context} = makeInterface();
	const calls = [];
	const callbackInfoPtr = makeCallbackInfo(mem, (...args) => {
		onCall?.({mem, iface, context, calls, args});
		calls.push(args);
	});
	assert.equal(mem.loadPtr(callbackInfoPtr + 8), 1, `${name}: callback slot was not written`);
	const callbackInfo = iface.CallbackInfo(callbackInfoPtr);
	assert.equal(typeof callbackInfo.callback, "function", `${name}: callback info did not resolve to a function`);
	await invoke({mem, iface, context, callbackInfoPtr});
	await settle();
	assert.equal(calls.length, 1, `${name}: expected exactly one terminal callback`);
	assertTerminalCallback(calls[0], expectedArgc, expectedStatus);
	callbackReader?.({mem, iface, calls});
}

async function main() {
	{
		const {mem, iface} = makeInterface();
		const surfaceTexturePtr = mem.exports.wgpu_alloc(12);
		mem.bytes.fill(0xaa, surfaceTexturePtr, surfaceTexturePtr + 12);

		iface.surfaces.get = () => ({
			getContext: () => ({
				getCurrentTexture: () => ({label: "surface-texture"}),
			}),
		});
		iface.textures.create = () => 104;

		assert.equal(iface.wgpuSurfaceGetCurrentTexture(1, surfaceTexturePtr), STATUS_SUCCESS);
		assert.equal(mem.loadPtr(surfaceTexturePtr + 0), 0);
		assert.equal(mem.loadPtr(surfaceTexturePtr + 4), 104);
		assert.equal(mem.loadPtr(surfaceTexturePtr + 8), STATUS_SUCCESS);
		assert.equal(iface.wgpuSurfacePresent(1), STATUS_SUCCESS);
	}

	let mapRejectMessagePtr = 0;
	await expectPromiseCallback(
		"mapAsync reject",
		async ({iface, context, callbackInfoPtr}) => {
			iface.buffers.get = () => ({
				buffer: {
					mapAsync: () => Promise.reject(new context.Error("map failed")),
				},
			});
			iface.wgpuBufferMapAsync(1, 1, 0, 0, callbackInfoPtr);
		},
		STATUS_ERROR,
		4,
		({mem, calls}) => {
			const args = calls[0];
			mapRejectMessagePtr = args[1];
			assert.equal(readStringView(mem, mapRejectMessagePtr).text, "map failed");
			assert.equal(mem.freed.includes(mapRejectMessagePtr), true);
		},
	);

	await expectPromiseCallback(
		"mapAsync success",
		async ({iface, callbackInfoPtr}) => {
			iface.buffers.get = () => ({
				buffer: {
					mapAsync: () => Promise.resolve(),
				},
			});
			iface.wgpuBufferMapAsync(1, 1, 0, 0, callbackInfoPtr);
		},
		STATUS_SUCCESS,
		4,
		({mem, calls}) => {
			const args = calls[0];
			const messagePtr = args[1];
			assert.equal(readStringView(mem, messagePtr).length, 0);
		},
	);

	let queueRejectMessagePtr = 0;
	await expectPromiseCallback(
		"queue work done reject",
		async ({iface, context, callbackInfoPtr}) => {
			context.console.warn = () => {};
			iface.queues.get = () => ({
				onSubmittedWorkDone: () => Promise.reject(new context.Error("queue failed")),
			});
			iface.wgpuQueueOnSubmittedWorkDone(1, callbackInfoPtr);
		},
		STATUS_ERROR,
		4,
		({mem, calls}) => {
			const args = calls[0];
			queueRejectMessagePtr = args[1];
			assert.equal(readStringView(mem, queueRejectMessagePtr).text, "queue failed");
			assert.equal(mem.freed.includes(queueRejectMessagePtr), true);
		},
	);

	await expectPromiseCallback(
		"queue work done success",
		async ({iface, callbackInfoPtr}) => {
			iface.queues.get = () => ({
				onSubmittedWorkDone: () => Promise.resolve(),
			});
			iface.wgpuQueueOnSubmittedWorkDone(1, callbackInfoPtr);
		},
		STATUS_SUCCESS,
		4,
		({mem, calls}) => {
			const args = calls[0];
			assert.equal(readStringView(mem, args[1]).length, 0);
		},
	);

	let adapterRejectMessagePtr = 0;
	await expectPromiseCallback(
		"request adapter reject",
		async ({iface, context, callbackInfoPtr}) => {
			context.navigator.gpu = {
				requestAdapter: () => Promise.reject(new context.Error("adapter failed")),
			};
			iface.wgpuInstanceRequestAdapter(1, 0, callbackInfoPtr);
		},
		STATUS_REQUEST_ADAPTER_ERROR,
		5,
		({mem, calls}) => {
			const args = calls[0];
			adapterRejectMessagePtr = args[2];
			assert.equal(args[1], 0);
			assert.equal(readStringView(mem, adapterRejectMessagePtr).text, "adapter failed");
			assert.equal(mem.freed.includes(adapterRejectMessagePtr), true);
		},
	);

	await expectPromiseCallback(
		"request adapter success",
		async ({iface, context, callbackInfoPtr}) => {
			context.navigator.gpu = {
				requestAdapter: () => Promise.resolve({label: "adapter"}),
			};
			iface.wgpuInstanceRequestAdapter(1, 0, callbackInfoPtr);
		},
		STATUS_SUCCESS,
		5,
		({mem, calls}) => {
			const args = calls[0];
			assert.notEqual(args[1], 0);
			assert.equal(readStringView(mem, args[2]).length, 0);
		},
	);

	let deviceRejectMessagePtr = 0;
	await expectPromiseCallback(
		"request device reject",
		async ({iface, context, callbackInfoPtr}) => {
			const adapter = {
				requestDevice: () => Promise.reject(new context.Error("device failed")),
			};
			iface.adapters.get = () => adapter;
			iface.wgpuAdapterRequestDevice(1, 0, callbackInfoPtr);
		},
		STATUS_ERROR,
		5,
		({mem, calls}) => {
			const args = calls[0];
			deviceRejectMessagePtr = args[2];
			assert.equal(args[1], 0);
			assert.equal(readStringView(mem, deviceRejectMessagePtr).text, "device failed");
			assert.equal(mem.freed.includes(deviceRejectMessagePtr), true);
		},
	);

	await expectPromiseCallback(
		"request device success",
		async ({iface, callbackInfoPtr}) => {
			const device = {
				requested: true,
			};
			const adapter = {
				requestDevice: () => Promise.resolve(device),
			};
			iface.adapters.get = () => adapter;
			iface.devices.create = () => 77;
			iface.wgpuAdapterRequestDevice(1, 0, callbackInfoPtr);
		},
		STATUS_SUCCESS,
		5,
		({mem, calls}) => {
			const args = calls[0];
			assert.equal(args[1], 77);
			assert.equal(readStringView(mem, args[2]).length, 0);
		},
	);

	{
		const {mem, iface, context} = makeInterface();
		const outerCalls = [];
		const uncapturedCalls = [];
		let devicePtrSeen = 0;
		let messagePtrSeen = 0;
		let createdDevice = null;
		const descriptorPtr = makeRequestDeviceDescriptor(mem, {
			uncapturedErrorHandler: (...args) => {
				uncapturedCalls.push(args);
				devicePtrSeen = args[0];
				messagePtrSeen = args[2];
				assert.equal(args.length, 5, "uncaptured error callback argc");
				assert.equal(args[0] > 0, true, "uncaptured error callback device pointer");
				assert.equal(args[1], STATUS_VALIDATION_ERROR, "uncaptured error callback status");
				assert.equal(readStringView(mem, args[2]).text, "boom");
				assert.equal(mem.freed.includes(args[0]), false);
				assert.equal(mem.freed.includes(args[2]), false);
			},
		});
		const callbackInfoPtr = makeCallbackInfo(mem, (...args) => outerCalls.push(args));
		iface.adapters.get = () => ({
			requestDevice: () => Promise.resolve({}),
		});
		iface.devices.create = (device) => {
			createdDevice = device;
			return 77;
		};
		iface.wgpuAdapterRequestDevice(1, descriptorPtr, callbackInfoPtr);
		await settle();
		assert.equal(outerCalls.length, 1);
		assert.equal(outerCalls[0].length, 5);
		assert.equal(outerCalls[0][1], 77);
		assert.equal(readStringView(mem, outerCalls[0][2]).length, 0);
		assert.equal(typeof createdDevice.onuncapturederror, "function");
		createdDevice.onuncapturederror({error: new context.GPUValidationError("boom")});
		assert.equal(uncapturedCalls.length, 1);
		assert.equal(mem.loadPtr(devicePtrSeen), 77);
		assert.equal(mem.freed.includes(devicePtrSeen), true);
		assert.equal(mem.freed.includes(messagePtrSeen), true);
	}

	{
		const {mem, iface} = makeInterface();
		const outerCalls = [];
		let createdDevice = null;
		const descriptorPtr = makeRequestDeviceDescriptor(mem);
		const callbackInfoPtr = makeCallbackInfo(mem, (...args) => outerCalls.push(args));
		iface.adapters.get = () => ({
			requestDevice: () => Promise.resolve({}),
		});
		iface.devices.create = (device) => {
			createdDevice = device;
			return 78;
		};
		iface.wgpuAdapterRequestDevice(1, descriptorPtr, callbackInfoPtr);
		await settle();
		assert.equal(outerCalls.length, 1);
		assert.equal(outerCalls[0].length, 5);
		assert.equal(outerCalls[0][1], 78);
		assert.equal(readStringView(mem, outerCalls[0][2]).length, 0);
		assert.equal(createdDevice.onuncapturederror, undefined);
	}

	await expectPromiseCallback(
		"compute pipeline reject",
		async ({iface, context, callbackInfoPtr}) => {
			iface.devices.get = () => ({
				createComputePipelineAsync: () => Promise.reject(new context.Error("compute failed")),
			});
			iface.ComputePipelineDescriptor = () => ({});
			iface.wgpuDeviceCreateComputePipelineAsync(1, 1, callbackInfoPtr);
		},
		STATUS_ERROR,
		5,
		({mem, calls}) => {
			const args = calls[0];
			const messagePtr = args[2];
			assert.equal(args[1], 0);
			assert.equal(readStringView(mem, messagePtr).text, "compute failed");
		},
	);

	await expectPromiseCallback(
		"compute pipeline success",
		async ({iface, callbackInfoPtr}) => {
			iface.devices.get = () => ({
				createComputePipelineAsync: () => Promise.resolve({label: "compute"}),
			});
			iface.computePipelines.create = () => 11;
			iface.ComputePipelineDescriptor = () => ({});
			iface.wgpuDeviceCreateComputePipelineAsync(1, 1, callbackInfoPtr);
		},
		STATUS_SUCCESS,
		5,
		({mem, calls}) => {
			const args = calls[0];
			assert.equal(args[1], 11);
			assert.equal(readStringView(mem, args[2]).length, 0);
		},
	);

	await expectPromiseCallback(
		"render pipeline reject",
		async ({iface, context, callbackInfoPtr}) => {
			iface.devices.get = () => ({
				createRenderPipelineAsync: () => Promise.reject(new context.Error("render failed")),
			});
			iface.RenderPipelineDescriptor = () => ({});
			iface.wgpuDeviceCreateRenderPipelineAsync(1, 1, callbackInfoPtr);
		},
		STATUS_ERROR,
		5,
		({mem, calls}) => {
			const args = calls[0];
			const messagePtr = args[2];
			assert.equal(args[1], 0);
			assert.equal(readStringView(mem, messagePtr).text, "render failed");
		},
	);

	await expectPromiseCallback(
		"render pipeline success",
		async ({iface, callbackInfoPtr}) => {
			iface.devices.get = () => ({
				createRenderPipelineAsync: () => Promise.resolve({label: "render"}),
			});
			iface.renderPipelines.create = () => 12;
			iface.RenderPipelineDescriptor = () => ({});
			iface.wgpuDeviceCreateRenderPipelineAsync(1, 1, callbackInfoPtr);
		},
		STATUS_SUCCESS,
		5,
		({mem, calls}) => {
			const args = calls[0];
			assert.equal(args[1], 12);
			assert.equal(readStringView(mem, args[2]).length, 0);
		},
	);

	{
		const {iface} = makeInterface();
		const filters = [];
		iface.devices.get = () => ({
			pushErrorScope: (filter) => filters.push(filter),
		});
		for (const filterInt of [1, 2, 3]) {
			iface.wgpuDevicePushErrorScope(1, filterInt);
		}
		assert.deepEqual(filters, ["validation", "out-of-memory", "internal"]);
	}

	await expectPromiseCallback(
		"pop error scope reject",
		async ({iface, context, callbackInfoPtr}) => {
			iface.devices.get = () => ({
				popErrorScope: () => Promise.reject(new context.Error("scope failed")),
			});
			iface.wgpuDevicePopErrorScope(1, callbackInfoPtr);
		},
		STATUS_ERROR,
		5,
		({mem, calls}) => {
			const args = calls[0];
			const messagePtr = args[2];
			assert.equal(args[1], STATUS_UNKNOWN_ERROR);
			assert.equal(readStringView(mem, messagePtr).text, "scope failed");
			assert.equal(mem.freed.includes(messagePtr), true);
		},
	);

	{
		const {mem, iface} = makeInterface();
		const calls = [];
		let zeroMessagePtr = 0;
		const callbackInfoPtr = makeCallbackInfo(mem, (...args) => calls.push(args));
		iface.devices.get = () => ({
			popErrorScope: () => Promise.resolve(null),
		});
		iface.wgpuDevicePopErrorScope(1, callbackInfoPtr);
		await settle();
		assert.equal(calls.length, 1);
		assertTerminalCallback(calls[0], 5, STATUS_SUCCESS);
		assert.equal(calls[0][1], 1);
		zeroMessagePtr = calls[0][2];
		assert.equal(readStringView(mem, zeroMessagePtr).length, 0);

		const probeSize = iface.sizes.StringView[0];
		const probePtr = mem.exports.wgpu_alloc(probeSize);
		assert.notEqual(probePtr, zeroMessagePtr);
		mem.storeI32(probePtr, 0xdeadbeef);
		mem.storeUint(probePtr + mem.intSize, 123);
		mem.exports.wgpu_free(probePtr);
		const reusedProbePtr = mem.exports.wgpu_alloc(probeSize);
		assert.equal(reusedProbePtr, probePtr);
		assert.equal(readStringView(mem, zeroMessagePtr).length, 0);

		const queueCalls = [];
		const queueCallbackInfoPtr = makeCallbackInfo(mem, (...args) => queueCalls.push(args));
		iface.queues.get = () => ({
			onSubmittedWorkDone: () => Promise.resolve(),
		});
		iface.wgpuQueueOnSubmittedWorkDone(1, queueCallbackInfoPtr);
		await settle();
		assert.equal(queueCalls.length, 1);
		assertTerminalCallback(queueCalls[0], 4, STATUS_SUCCESS);
		assert.equal(queueCalls[0][1], zeroMessagePtr);
		assert.equal(readStringView(mem, queueCalls[0][1]).length, 0);
		mem.exports.wgpu_free(reusedProbePtr);
	}

	{
		const {mem, iface, context} = makeInterface();
		const calls = [];
		const callbackInfoPtr = makeCallbackInfo(mem, (...args) => calls.push(args));
		iface.devices.get = () => ({
			popErrorScope: () => Promise.resolve(new context.GPUValidationError("")),
		});
		iface.wgpuDevicePopErrorScope(1, callbackInfoPtr);
		await settle();
		assert.equal(calls.length, 1);
		assertTerminalCallback(calls[0], 5, STATUS_SUCCESS);
		assert.equal(calls[0][1], STATUS_VALIDATION_ERROR);
		const zeroMessagePtr = calls[0][2];
		assert.equal(readStringView(mem, zeroMessagePtr).length, 0);
		assert.equal(mem.freed.includes(zeroMessagePtr), false);

		const probeSize = iface.sizes.StringView[0];
		const probePtr = mem.exports.wgpu_alloc(probeSize);
		assert.notEqual(probePtr, zeroMessagePtr);
		mem.storeI32(probePtr, 0xdeadbeef);
		mem.storeUint(probePtr + mem.intSize, 123);
		mem.exports.wgpu_free(probePtr);
		const reusedProbePtr = mem.exports.wgpu_alloc(probeSize);
		assert.equal(reusedProbePtr, probePtr);
		assert.equal(readStringView(mem, zeroMessagePtr).length, 0);

		const queueCalls = [];
		const queueCallbackInfoPtr = makeCallbackInfo(mem, (...args) => queueCalls.push(args));
		iface.queues.get = () => ({
			onSubmittedWorkDone: () => Promise.resolve(),
		});
		iface.wgpuQueueOnSubmittedWorkDone(1, queueCallbackInfoPtr);
		await settle();
		assert.equal(queueCalls.length, 1);
		assertTerminalCallback(queueCalls[0], 4, STATUS_SUCCESS);
		assert.equal(queueCalls[0][1], zeroMessagePtr);
		assert.equal(readStringView(mem, queueCalls[0][1]).length, 0);
		mem.exports.wgpu_free(reusedProbePtr);
	}

	await expectPromiseCallback(
		"pop error scope error",
		async ({iface, context, callbackInfoPtr}) => {
			iface.devices.get = () => ({
				popErrorScope: () => Promise.resolve(new context.GPUValidationError("scope validation")),
			});
			iface.wgpuDevicePopErrorScope(1, callbackInfoPtr);
		},
		STATUS_SUCCESS,
		5,
		({mem, calls}) => {
			const args = calls[0];
			const messagePtr = args[2];
			assert.equal(args[1], STATUS_VALIDATION_ERROR);
			assert.equal(readStringView(mem, messagePtr).text, "scope validation");
		},
	);

	await expectPromiseCallback(
		"compilation info reject",
		async ({iface, context, callbackInfoPtr}) => {
			context.console.warn = () => {};
			iface.shaderModules.get = () => ({
				getCompilationInfo: () => Promise.reject(new context.Error("compilation failed")),
			});
			iface.wgpuShaderModuleGetCompilationInfo(1, callbackInfoPtr);
		},
		2,
		4,
		({mem, calls}) => {
			const args = calls[0];
			assert.equal(args[1], 0);
			assert.equal(args[2], CALLBACK_USERDATA1);
			assert.equal(args[3], CALLBACK_USERDATA2);
			assert.equal(readStringView(mem, args[1]).length, 0);
		},
	);

	await expectPromiseCallback(
		"compilation info success",
		async ({iface, callbackInfoPtr}) => {
			iface.shaderModules.get = () => ({
				getCompilationInfo: () => Promise.resolve({
					messages: [
						{message: "hello", type: "info", lineNum: 1n, linePos: 2n, offset: 3n, length: 4n},
					],
				}),
			});
			iface.wgpuShaderModuleGetCompilationInfo(1, callbackInfoPtr);
		},
		STATUS_SUCCESS,
		4,
		({calls}) => {
			const args = calls[0];
			assert.notEqual(args[1], 0);
		},
	);

	console.log("wgpu bridge regression probe passed");
}

main().catch((error) => {
	console.error(error);
	process.exitCode = 1;
});
