import '../styles.css';

type ProcessResourceInfo = {
  pid: number;
  name: string;
  state: string;
  cpu: number;
  memory: number;
};

type ResourceMetrics = {
  cpu: { usage: number };
  memory: { total: number; used: number; usage: number };
  swap: { total: number; used: number; usage: number };
  loadAverage: [number, number, number];
  uptime: number;
  processes: ProcessResourceInfo[];
};

type DashboardElements = {
  connectionStatus: HTMLDivElement;
  cpuValue: HTMLElement;
  cpuBar: HTMLElement;
  memoryValue: HTMLElement;
  memoryDetails: HTMLElement;
  memoryBar: HTMLElement;
  swapValue: HTMLElement;
  swapDetails: HTMLElement;
  swapBar: HTMLElement;
  loadValue: HTMLElement;
  uptimeValue: HTMLElement;
  processTable: HTMLTableSectionElement;
};

const RECONNECT_DELAY_MILLISECONDS = 1500;
const WEB_SOCKET_URL = 'ws://localhost:9002/ws';

function requireElement<T extends Element>(selector: string): T {
  const element = document.querySelector<T>(selector);

  if (element === null) {
    throw new Error(`Missing DOM element: ${selector}`);
  }

  return element;
}

const dashboardElements: DashboardElements = {
  connectionStatus: requireElement<HTMLDivElement>('#connectionStatus'),
  cpuValue: requireElement<HTMLElement>('#cpuValue'),
  cpuBar: requireElement<HTMLElement>('#cpuBar'),
  memoryValue: requireElement<HTMLElement>('#memoryValue'),
  memoryDetails: requireElement<HTMLElement>('#memoryDetails'),
  memoryBar: requireElement<HTMLElement>('#memoryBar'),
  swapValue: requireElement<HTMLElement>('#swapValue'),
  swapDetails: requireElement<HTMLElement>('#swapDetails'),
  swapBar: requireElement<HTMLElement>('#swapBar'),
  loadValue: requireElement<HTMLElement>('#loadValue'),
  uptimeValue: requireElement<HTMLElement>('#uptimeValue'),
  processTable: requireElement<HTMLTableSectionElement>('#processTable'),
};

function formatPercent(value: number): string {
  return `${value.toFixed(1)}%`;
}

function formatUptime(seconds: number): string {
  const days = Math.floor(seconds / 86400);
  const hours = Math.floor((seconds % 86400) / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);

  if (days > 0) {
    return `${days}d ${hours}h ${minutes}m`;
  }

  if (hours > 0) {
    return `${hours}h ${minutes}m`;
  }

  return `${minutes}m`;
}

function setConnectionStatus(isConnected: boolean): void {
  dashboardElements.connectionStatus.textContent = isConnected ? 'Connected' : 'Disconnected';
  dashboardElements.connectionStatus.classList.toggle('status--online', isConnected);
  dashboardElements.connectionStatus.classList.toggle('status--offline', !isConnected);
}

function setProgressBarValue(progressBarElement: HTMLElement, value: number): void {
  const safeValue = Math.max(0, Math.min(100, value));
  progressBarElement.style.width = `${safeValue}%`;
}

function renderProcessTable(processes: ProcessResourceInfo[]): void {
  const processRows = processes.map((processInfo) => {
    const rowElement = document.createElement('tr');

    rowElement.innerHTML = `
      <td>${processInfo.pid}</td>
      <td>${processInfo.name}</td>
      <td>${processInfo.state}</td>
      <td>${formatPercent(processInfo.cpu)}</td>
      <td>${processInfo.memory} MB</td>
    `;

    return rowElement;
  });

  dashboardElements.processTable.replaceChildren(...processRows);
}

function renderResourceMetrics(metrics: ResourceMetrics): void {
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
    .join('  ');
  dashboardElements.uptimeValue.textContent = `uptime: ${formatUptime(metrics.uptime)}`;

  renderProcessTable(metrics.processes);
}

function connectToBackend(): void {
  const webSocket = new WebSocket(WEB_SOCKET_URL);

  webSocket.addEventListener('open', () => {
    setConnectionStatus(true);
  });

  webSocket.addEventListener('close', () => {
    setConnectionStatus(false);
    window.setTimeout(connectToBackend, RECONNECT_DELAY_MILLISECONDS);
  });

  webSocket.addEventListener('error', () => {
    setConnectionStatus(false);
  });

  webSocket.addEventListener('message', (event: MessageEvent<string>) => {
    try {
      const metrics = JSON.parse(event.data) as ResourceMetrics;
      renderResourceMetrics(metrics);
    } catch (error) {
      console.error('Failed to parse backend payload', error);
    }
  });
}

connectToBackend();
