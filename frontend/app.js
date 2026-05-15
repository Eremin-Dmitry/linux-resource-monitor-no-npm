const RECONNECT_DELAY_MILLISECONDS = 1500;
const WEB_SOCKET_URL = "ws://localhost:9002/ws";

function requireElement(selector)
{
    const element = document.querySelector(selector);

    if (element === null)
    {
        throw new Error(`Missing DOM element: ${selector}`);
    }

    return element;
}

const dashboardElements =
{
    connectionStatus: requireElement("#connectionStatus"),
    cpuValue: requireElement("#cpuValue"),
    cpuBar: requireElement("#cpuBar"),
    memoryValue: requireElement("#memoryValue"),
    memoryDetails: requireElement("#memoryDetails"),
    memoryBar: requireElement("#memoryBar"),
    swapValue: requireElement("#swapValue"),
    swapDetails: requireElement("#swapDetails"),
    swapBar: requireElement("#swapBar"),
    loadValue: requireElement("#loadValue"),
    uptimeValue: requireElement("#uptimeValue"),
    processTable: requireElement("#processTable")
};

function formatPercent(value)
{
    return `${value.toFixed(1)}%`;
}

function formatUptime(seconds)
{
    const days = Math.floor(seconds / 86400);
    const hours = Math.floor((seconds % 86400) / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);

    if (days > 0)
    {
        return `${days}d ${hours}h ${minutes}m`;
    }

    if (hours > 0)
    {
        return `${hours}h ${minutes}m`;
    }

    return `${minutes}m`;
}

function setConnectionStatus(isConnected)
{
    dashboardElements.connectionStatus.textContent = isConnected ? "Connected" : "Disconnected";
    dashboardElements.connectionStatus.classList.toggle("status--online", isConnected);
    dashboardElements.connectionStatus.classList.toggle("status--offline", !isConnected);
}

function setProgressBarValue(progressBarElement, value)
{
    const safeValue = Math.max(0, Math.min(100, value));
    progressBarElement.style.width = `${safeValue}%`;
}

function escapeHtml(value)
{
    return String(value)
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#039;");
}

function renderProcessTable(processes)
{
    const processRows = processes.map((processInfo) =>
    {
        const rowElement = document.createElement("tr");

        rowElement.innerHTML = `
            <td>${processInfo.pid}</td>
            <td>${escapeHtml(processInfo.name)}</td>
            <td>${escapeHtml(processInfo.state)}</td>
            <td>${formatPercent(processInfo.cpu)}</td>
            <td>${processInfo.memory} MB</td>
        `;

        return rowElement;
    });

    dashboardElements.processTable.replaceChildren(...processRows);
}

function renderResourceMetrics(metrics)
{
    dashboardElements.cpuValue.textContent = formatPercent(metrics.cpu.usage);
    setProgressBarValue(dashboardElements.cpuBar, metrics.cpu.usage);

    dashboardElements.memoryValue.textContent = formatPercent(metrics.memory.usage);
    dashboardElements.memoryDetails.textContent = `${metrics.memory.used} / ${metrics.memory.total} MB`;
    setProgressBarValue(dashboardElements.memoryBar, metrics.memory.usage);

    dashboardElements.swapValue.textContent = formatPercent(metrics.swap.usage);
    dashboardElements.swapDetails.textContent = `${metrics.swap.used} / ${metrics.swap.total} MB`;
    setProgressBarValue(dashboardElements.swapBar, metrics.swap.usage);

    dashboardElements.loadValue.textContent = metrics.loadAverage
        .map((loadAverageValue) => loadAverageValue.toFixed(2))
        .join("  ");
    dashboardElements.uptimeValue.textContent = `uptime: ${formatUptime(metrics.uptime)}`;

    renderProcessTable(metrics.processes);
}

function connectToBackend()
{
    const webSocket = new WebSocket(WEB_SOCKET_URL);

    webSocket.addEventListener("open", () =>
    {
        setConnectionStatus(true);
    });

    webSocket.addEventListener("close", () =>
    {
        setConnectionStatus(false);
        window.setTimeout(connectToBackend, RECONNECT_DELAY_MILLISECONDS);
    });

    webSocket.addEventListener("error", () =>
    {
        setConnectionStatus(false);
    });

    webSocket.addEventListener("message", (event) =>
    {
        try
        {
            const metrics = JSON.parse(event.data);
            renderResourceMetrics(metrics);
        }
        catch (error)
        {
            console.error("Failed to parse backend payload", error);
        }
    });
}

connectToBackend();
